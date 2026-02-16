import { invoke } from "@tauri-apps/api/core";
import { listen } from "@tauri-apps/api/event";

const ui = {
  nameInput: document.getElementById("nameInput"),
  targetInput: document.getElementById("targetInput"),
  iconInput: document.getElementById("iconInput"),
  argsInput: document.getElementById("argsInput"),
  itemTypeSelect: document.getElementById("itemTypeSelect"),
  saveBtn: document.getElementById("saveBtn"),
  cancelBtn: document.getElementById("cancelBtn"),
  status: document.getElementById("editorStatus")
};

const editorState = {
  groupId: null,
  item: null
};

function setStatus(message, isError = false) {
  ui.status.textContent = message || "";
  ui.status.style.color = isError ? "#b91c1c" : "#0f766e";
}

function fillForm() {
  const item = editorState.item;
  ui.nameInput.value = item?.name || "";
  ui.targetInput.value = item?.targetPath || "";
  ui.iconInput.value = item?.iconLocation || "";
  ui.argsInput.value = item?.arguments || "";
  ui.itemTypeSelect.value = item?.itemType || "app";
}

async function loadContext() {
  const ctx = await invoke("get_editor_context");
  if (!ctx) {
    setStatus("未获取到编辑上下文", true);
    return;
  }
  editorState.groupId = ctx.groupId;
  editorState.item = ctx.item || null;
  fillForm();
}

async function saveItem() {
  const name = ui.nameInput.value.trim();
  const itemType = ui.itemTypeSelect.value;
  const targetPath = ui.targetInput.value.trim();
  const iconLocation = ui.iconInput.value.trim();

  if (!name) {
    setStatus("名称不能为空", true);
    return;
  }

  if (itemType === "app" && !targetPath) {
    setStatus("应用类型必须填写目标路径", true);
    return;
  }

  const payload = {
    id: editorState.item?.id || null,
    itemType,
    name,
    targetPath,
    iconLocation,
    arguments: ui.argsInput.value || "",
    enabled: true
  };

  try {
    await invoke("upsert_item", {
      groupId: editorState.groupId,
      item: payload
    });
    setStatus("保存成功");
    await invoke("close_editor");
  } catch (error) {
    setStatus(String(error), true);
  }
}

function bindEvents() {
  document.addEventListener("contextmenu", (event) => {
    event.preventDefault();
  });

  ui.saveBtn.addEventListener("click", saveItem);
  ui.cancelBtn.addEventListener("click", () => invoke("close_editor"));
  document.addEventListener("keydown", (event) => {
    if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === "s") {
      event.preventDefault();
      saveItem();
    }
  });
}

async function registerEvents() {
  await listen("editor-context-changed", async () => {
    await loadContext();
  });
}

(async function bootstrap() {
  bindEvents();
  try {
    await registerEvents();
    await loadContext();
  } catch (error) {
    setStatus(`初始化失败: ${String(error)}`, true);
  }
})();
