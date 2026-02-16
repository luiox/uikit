<template>
  <div class="app-window" @contextmenu.prevent>
    <header class="top-bar">
      <div class="top-title">nassistant</div>
      <button class="hide-btn" @click="onHide">×</button>
    </header>

    <div class="body-layout">
      <aside class="group-panel" @contextmenu.prevent="openGroupMenu">
        <div class="section-title">分组</div>
        <ul class="group-list">
          <li
            v-for="group in groups"
            :key="group.id"
            class="group-item"
            :class="{ active: group.id === activeGroupId }"
            @click="selectGroup(group.id)"
          >
            {{ group.name }}
          </li>
        </ul>
      </aside>

      <main class="item-panel" @contextmenu.prevent="openItemMenu">
        <div class="section-title">启动项</div>
        <ul class="item-list">
          <li
            v-for="item in filteredItems"
            :key="item.id"
            class="item-row"
            :class="{ active: item.id === selectedItemId, separator: item.itemType === 'separator' }"
            @click="selectedItemId = item.id"
            @dblclick="item.itemType !== 'separator' && onLaunch()"
          >
            <span class="item-icon">{{ item.itemType === 'separator' ? '--' : (item.name?.[0] || '?') }}</span>
            <span class="item-name">{{ item.name || '(未命名)' }}</span>
            <span class="item-count">{{ item.launchCount ?? 0 }}</span>
          </li>
        </ul>
      </main>
    </div>

    <footer class="status-line" :class="{ error: isError }">{{ statusText }}</footer>

    <div
      v-if="groupMenu.visible"
      class="context-menu"
      :style="{ left: `${groupMenu.x}px`, top: `${groupMenu.y}px` }"
    >
      <button class="menu-item" @click="onAddGroup">添加分组</button>
    </div>

    <div
      v-if="itemMenu.visible"
      class="context-menu"
      :style="{ left: `${itemMenu.x}px`, top: `${itemMenu.y}px` }"
    >
      <button class="menu-item" @click="onAddItem">添加启动项</button>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed, onMounted, ref } from 'vue';
import {
  addGroup,
  deleteItem,
  hideCurrentWindow,
  launchItem,
  loadLauncherState,
  onDataChanged,
  onSettingsChanged,
  openEditor
} from './api';
import type { Group, LaunchItem } from './types';

const groups = ref<Group[]>([]);
const activeGroupId = ref<string | null>(null);
const selectedItemId = ref<string | null>(null);
const searchKeyword = ref('');
const statusText = ref('');
const isError = ref(false);
const groupMenu = ref({ visible: false, x: 0, y: 0 });
const itemMenu = ref({ visible: false, x: 0, y: 0 });

const activeGroup = computed(() => groups.value.find((g) => g.id === activeGroupId.value) ?? null);
const selectedItem = computed<LaunchItem | null>(() => activeGroup.value?.items.find((it) => it.id === selectedItemId.value) ?? null);

const filteredItems = computed(() => {
  const list = activeGroup.value?.items ?? [];
  const key = searchKeyword.value.trim().toLowerCase();
  if (!key) return list;
  return list.filter((it) => (it.name ?? '').toLowerCase().includes(key));
});

function setStatus(message: string, error = false) {
  statusText.value = message;
  isError.value = error;
}

function selectGroup(id: string) {
  activeGroupId.value = id;
  selectedItemId.value = null;
}

async function refresh() {
  const state = await loadLauncherState();
  groups.value = state.groups ?? [];

  const preferred = state.settings?.currentGroup;
  const byName = groups.value.find((g) => g.name === preferred);
  activeGroupId.value = byName?.id ?? groups.value[0]?.id ?? null;
  selectedItemId.value = null;
}

function closeMenus() {
  groupMenu.value.visible = false;
  itemMenu.value.visible = false;
}

function openGroupMenu(event: MouseEvent) {
  closeMenus();
  groupMenu.value = { visible: true, x: event.clientX, y: event.clientY };
}

function openItemMenu(event: MouseEvent) {
  closeMenus();
  itemMenu.value = { visible: true, x: event.clientX, y: event.clientY };
}

async function onAddGroup() {
  closeMenus();
  const name = window.prompt('输入新分组名称');
  if (!name) return;
  try {
    await addGroup(name.trim());
    setStatus('分组已添加');
    await refresh();
  } catch (error) {
    setStatus(String(error), true);
  }
}

async function onAddItem() {
  closeMenus();
  if (!activeGroup.value) {
    setStatus('没有可用分组', true);
    return;
  }
  try {
    await openEditor(activeGroup.value.id, null);
  } catch (error) {
    setStatus(`打开编辑窗口失败: ${String(error)}`, true);
  }
}

async function onDelete() {
  if (!activeGroup.value || !selectedItem.value) {
    setStatus('请先选择要删除的条目', true);
    return;
  }
  if (!window.confirm(`确认删除：${selectedItem.value.name} ?`)) {
    return;
  }
  try {
    await deleteItem(activeGroup.value.id, selectedItem.value.id);
    setStatus('删除成功');
    await refresh();
  } catch (error) {
    setStatus(String(error), true);
  }
}

async function onLaunch() {
  if (!activeGroup.value || !selectedItem.value) {
    setStatus('请先选择要启动的条目', true);
    return;
  }
  if (selectedItem.value.itemType === 'separator') {
    setStatus('分隔项不能启动', true);
    return;
  }
  try {
    const result = await launchItem(activeGroup.value.id, selectedItem.value.id);
    setStatus(result?.message ?? '启动成功');
    await refresh();
  } catch (error) {
    setStatus(String(error), true);
  }
}

async function onHide() {
  await hideCurrentWindow();
}

onMounted(async () => {
  await refresh();
  await onDataChanged(async () => {
    await refresh();
  });
  await onSettingsChanged(async () => {
    await refresh();
  });
  document.addEventListener('click', () => {
    closeMenus();
  });
  document.addEventListener('keydown', (event) => {
    if (event.key === 'Escape') {
      closeMenus();
    }
    if (event.key === 'Delete') {
      onDelete();
    }
    if (event.key === 'Enter') {
      onLaunch();
    }
  });
  setStatus('就绪');
});
</script>

<style scoped>
.app-window {
  height: 100vh;
  display: grid;
  grid-template-rows: 42px 1fr 28px;
  background: #eff3f8;
  color: #1f2937;
  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif;
}

.top-bar {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 8px 0 12px;
  background: linear-gradient(180deg, #fdfefe 0%, #eef3f9 100%);
  border-bottom: 1px solid #dbe3ef;
}

.top-title {
  font-size: 14px;
  font-weight: 600;
}

.hide-btn {
  width: 30px;
  height: 28px;
  border: 1px solid #ccd6e4;
  border-radius: 6px;
  background: #fff;
  font-size: 18px;
  line-height: 1;
  cursor: pointer;
}

.body-layout {
  display: grid;
  grid-template-columns: 220px 1fr;
  min-height: 0;
}

.group-panel,
.item-panel {
  display: flex;
  flex-direction: column;
  min-height: 0;
}

.group-panel {
  border-right: 1px solid #dbe3ef;
  background: #f8fbff;
}

.item-panel {
  background: #ffffff;
}

.section-title {
  padding: 10px 12px;
  border-bottom: 1px solid #e3eaf3;
  font-size: 13px;
  color: #516174;
  background: #f9fcff;
}

.group-list,
.item-list {
  list-style: none;
  margin: 0;
  padding: 8px;
  overflow: auto;
}

.group-item {
  padding: 8px 10px;
  border-radius: 7px;
  cursor: pointer;
  margin-bottom: 4px;
  border: 1px solid transparent;
  font-size: 13px;
}

.group-item:hover {
  background: #edf4ff;
  border-color: #d0def5;
}

.group-item.active {
  background: #2f67e6;
  color: #fff;
  border-color: #2f67e6;
}

.item-row {
  display: grid;
  grid-template-columns: 34px 1fr auto;
  align-items: center;
  gap: 10px;
  background: #fff;
  border: 1px solid #e6ebf2;
  border-radius: 8px;
  padding: 9px 10px;
  margin-bottom: 7px;
  cursor: pointer;
}

.item-row:hover {
  border-color: #b6c9f1;
  background: #f9fbff;
}

.item-row.active {
  border-color: #2f67e6;
  background: #edf3ff;
}

.item-row.separator {
  opacity: 0.72;
}

.item-icon {
  width: 26px;
  height: 26px;
  border-radius: 6px;
  display: grid;
  place-items: center;
  background: #edf2ff;
  color: #2f4a95;
  font-size: 12px;
  font-weight: 600;
}

.item-name {
  font-weight: 600;
  font-size: 13px;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.item-count {
  font-size: 12px;
  color: #455468;
}

.status-line {
  border-top: 1px solid #dbe3ef;
  background: #fdfefe;
  padding: 5px 10px;
  font-size: 13px;
  color: #0f766e;
}

.status-line.error {
  color: #b91c1c;
}

.context-menu {
  position: fixed;
  z-index: 9999;
  min-width: 150px;
  background: #fff;
  border: 1px solid #d0dae8;
  border-radius: 8px;
  box-shadow: 0 10px 26px rgba(15, 23, 42, 0.16);
  padding: 6px;
}

.menu-item {
  width: 100%;
  border: none;
  background: transparent;
  text-align: left;
  padding: 8px 10px;
  border-radius: 6px;
  font-size: 13px;
  cursor: pointer;
}

.menu-item:hover {
  background: #edf3ff;
}
</style>
