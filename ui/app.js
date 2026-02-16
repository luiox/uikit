import { invoke } from "@tauri-apps/api/core";
import { listen } from "@tauri-apps/api/event";

const ui = {
  groupList: document.getElementById("groupList"),
  itemList: document.getElementById("itemList"),
  searchInput: document.getElementById("searchInput"),
  addBtn: document.getElementById("addBtn"),
  editBtn: document.getElementById("editBtn"),
  deleteBtn: document.getElementById("deleteBtn"),
  launchBtn: document.getElementById("launchBtn"),
  executeHide: document.getElementById("executeHide"),
  hotkeyInput: document.getElementById("hotkeyInput"),
  saveSettingsBtn: document.getElementById("saveSettingsBtn"),
  statusText: document.getElementById("statusText")
};

const state = {
  groups: [],
  settings: null,
  activeGroupId: null,
  selectedItemId: null,
  searchKeyword: ""
};

function setStatus(message, isError = false) {
  ui.statusText.textContent = message || "";
  ui.statusText.style.color = isError ? "#b91c1c" : "#0f766e";
}

function getActiveGroup() {
  return state.groups.find((g) => g.id === state.activeGroupId) || null;
}

function getSelectedItem() {
  const group = getActiveGroup();
  if (!group) {
    return null;
  }
  return group.items.find((it) => it.id === state.selectedItemId) || null;
}

function renderGroups() {
  ui.groupList.innerHTML = "";
  state.groups.forEach((group) => {
    const li = document.createElement("li");
    li.className = `group-item ${group.id === state.activeGroupId ? "active" : ""}`;
    li.textContent = group.name;
    li.addEventListener("click", () => {
      state.activeGroupId = group.id;
      state.selectedItemId = null;
      renderGroups();
      renderItems();
    });
    ui.groupList.appendChild(li);
  });
}

function renderItems() {
  ui.itemList.innerHTML = "";
  const group = getActiveGroup();
  if (!group) {
    return;
  }

  const keyword = state.searchKeyword.trim().toLowerCase();
  const filtered = group.items.filter((item) => {
    if (!keyword) {
      return true;
    }
    return item.name.toLowerCase().includes(keyword);
  });

  for (const item of filtered) {
    const li = document.createElement("li");
    const isSeparator = item.itemType === "separator";
    li.className = `item ${item.id === state.selectedItemId ? "active" : ""} ${isSeparator ? "separator" : ""}`;

    li.innerHTML = `
      <span class="item-icon">${isSeparator ? "--" : (item.name[0] || "?")}</span>
      <span>
        <div class="item-name">${item.name || "(未命名)"}</div>
        <div class="item-sub">${isSeparator ? "分隔项" : (item.targetPath || "")}</div>
      </span>
      <span class="badge">${item.launchCount ?? 0}</span>
    `;

    li.addEventListener("click", () => {
      state.selectedItemId = item.id;
      renderItems();
    });

    li.addEventListener("dblclick", async () => {
      if (!isSeparator) {
        await launchSelectedItem();
      }
    });

    ui.itemList.appendChild(li);
  }
}

async function loadState() {
  const res = await invoke("load_launcher_state");
  state.groups = res.groups || [];
  state.settings = res.settings || { hotkey: "Alt+1", executeHide: true, currentGroup: null };

  const preferred = state.settings.currentGroup;
  const byName = state.groups.find((g) => g.name === preferred);
  state.activeGroupId = byName?.id || state.groups[0]?.id || null;
  state.selectedItemId = null;

  ui.executeHide.checked = !!state.settings.executeHide;
  ui.hotkeyInput.value = state.settings.hotkey || "Alt+1";

  renderGroups();
  renderItems();
}

async function openEditorForCurrent(editMode) {
  const group = getActiveGroup();
  if (!group) {
    setStatus("没有可用分组", true);
    return;
  }

  const selected = getSelectedItem();
  if (editMode && !selected) {
    setStatus("请先选择要编辑的条目", true);
    return;
  }

  try {
    await invoke("open_editor", {
      groupId: group.id,
      itemId: editMode ? selected.id : null
    });
  } catch (error) {
    setStatus(`打开编辑窗口失败: ${String(error)}`, true);
  }
}

async function deleteSelectedItem() {
  const group = getActiveGroup();
  const item = getSelectedItem();
  if (!group || !item) {
    setStatus("请先选择要删除的条目", true);
    return;
  }

  const ok = window.confirm(`确认删除：${item.name} ?`);
  if (!ok) {
    return;
  }

  await invoke("delete_item", {
    groupId: group.id,
    itemId: item.id
  });

  setStatus("删除成功");
  await loadState();
}

async function launchSelectedItem() {
  const group = getActiveGroup();
  const item = getSelectedItem();
  if (!group || !item) {
    setStatus("请先选择要启动的条目", true);
    return;
  }

  try {
    const result = await invoke("launch_item", {
      groupId: group.id,
      itemId: item.id
    });
    setStatus(result.message || "启动成功");
    await loadState();
  } catch (error) {
    setStatus(String(error), true);
  }
}

async function saveSettings() {
  try {
    const payload = {
      hotkey: ui.hotkeyInput.value.trim() || "Alt+1",
      executeHide: ui.executeHide.checked,
      currentGroup: getActiveGroup()?.name || null
    };

    await invoke("update_settings", { settings: payload });
    setStatus("设置已保存（重启后保证快捷键生效）");
    state.settings = payload;
  } catch (error) {
    setStatus(String(error), true);
  }
}

function bindEvents() {
  document.addEventListener("contextmenu", (event) => {
    event.preventDefault();
  });

  ui.searchInput.addEventListener("input", () => {
    state.searchKeyword = ui.searchInput.value;
    renderItems();
  });

  ui.addBtn.addEventListener("click", () => openEditorForCurrent(false));
  ui.editBtn.addEventListener("click", () => openEditorForCurrent(true));
  ui.deleteBtn.addEventListener("click", deleteSelectedItem);
  ui.launchBtn.addEventListener("click", launchSelectedItem);
  ui.saveSettingsBtn.addEventListener("click", saveSettings);

  document.addEventListener("keydown", (event) => {
    if (event.key === "Enter" && document.activeElement !== ui.searchInput) {
      launchSelectedItem();
    }
  });
}

async function registerEvents() {
  await listen("data-changed", async () => {
    await loadState();
  });
  await listen("settings-changed", async () => {
    await loadState();
  });
}

(async function bootstrap() {
  bindEvents();
  try {
    await registerEvents();
    await loadState();
    setStatus("就绪");
  } catch (error) {
    setStatus(`初始化失败: ${String(error)}`, true);
  }
})();
