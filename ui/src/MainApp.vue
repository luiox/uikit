<template>
  <div class="app-container" @contextmenu.prevent>
    <!-- 标题栏 -->
    <header class="title-bar">
      <div class="title">Poner</div>
      <div class="title-bar-actions">
        <button class="title-bar-btn" @click="toggleSearch">{{ showSearch ? '×' : '🔍' }}</button>
        <button class="title-bar-btn" @click="toggleMenu">☰</button>
        <button class="title-bar-btn close-btn" @click="closeApp">×</button>
      </div>
    </header>

    <!-- 搜索栏 -->
    <div v-if="showSearch" class="search-bar">
      <input v-model="searchKeyword" type="text" placeholder="搜索..." />
    </div>

    <!-- 主内容区 -->
    <div class="app-shell">
      <!-- 左侧分类栏 -->
      <aside class="category-panel">
        <ul class="category-list">
          <li
            v-for="group in groups"
            :key="group.id"
            class="category-item"
            :class="{ active: group.id === activeGroupId }"
            @click="selectGroup(group.id)"
          >
            {{ group.name }}
          </li>
        </ul>
      </aside>

      <!-- 右侧启动项区域 -->
      <main class="launcher-panel">
        <ul class="launch-item-list">
          <li
            v-for="item in filteredItems"
            :key="item.id"
            class="launch-item"
            :class="{ active: item.id === selectedItemId }"
            @click="selectedItemId = item.id"
            @dblclick="onLaunch()"
          >
            <span class="launch-item-icon">{{ item.name?.[0] || '?' }}</span>
            <span class="launch-item-name">{{ item.name || '(未命名)' }}</span>
          </li>
        </ul>
      </main>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed, onMounted, ref } from 'vue';
import {
  deleteItem,
  launchItem,
  loadLauncherState,
  onDataChanged,
  onSettingsChanged,
  openEditor,
  updateSettings
} from './api';
import type { Group, LaunchItem } from './types';

const groups = ref<Group[]>([]);
const activeGroupId = ref<string | null>(null);
const selectedItemId = ref<string | null>(null);
const searchKeyword = ref('');
const executeHide = ref(true);
const hotkey = ref('Alt+1');
const statusText = ref('');
const isError = ref(false);

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

  executeHide.value = !!state.settings?.executeHide;
  hotkey.value = state.settings?.hotkey ?? 'Alt+1';
}

async function onAdd() {
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

async function onEdit() {
  if (!activeGroup.value || !selectedItem.value) {
    setStatus('请先选择要编辑的条目', true);
    return;
  }
  try {
    await openEditor(activeGroup.value.id, selectedItem.value.id);
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

async function onSaveSettings() {
  try {
    await updateSettings({
      hotkey: hotkey.value?.trim() || 'Alt+1',
      executeHide: executeHide.value,
      currentGroup: activeGroup.value?.name ?? null
    });
    setStatus('设置已保存');
  } catch (error) {
    setStatus(String(error), true);
  }
}

onMounted(async () => {
  await refresh();
  await onDataChanged(async () => {
    await refresh();
  });
  await onSettingsChanged(async () => {
    await refresh();
  });
  setStatus('就绪');
});
</script>

<style scoped>
.app-shell {
  height: 100vh;
  display: grid;
  grid-template-columns: 240px 1fr;
  background: #f3f5f9;
  color: #1f2937;
  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif;
}

.group-panel {
  border-right: 1px solid #dfe4ec;
  background: linear-gradient(180deg, #ffffff 0%, #f8fafc 100%);
  display: flex;
  flex-direction: column;
}

.panel-title {
  padding: 14px 16px;
  font-size: 14px;
  font-weight: 600;
  border-bottom: 1px solid #e8edf4;
}

.group-list {
  list-style: none;
  margin: 0;
  padding: 8px;
  overflow: auto;
}

.group-item {
  padding: 10px 12px;
  border-radius: 8px;
  cursor: pointer;
  margin-bottom: 4px;
  border: 1px solid transparent;
}

.group-item:hover {
  background: #eef5ff;
  border-color: #cddffd;
}

.group-item.active {
  background: #2f67e6;
  color: #fff;
  border-color: #2f67e6;
}

.content-panel {
  display: grid;
  grid-template-rows: 58px 1fr 52px;
}

.toolbar {
  display: grid;
  grid-template-columns: 1fr auto auto auto auto;
  gap: 8px;
  padding: 10px;
  border-bottom: 1px solid #e2e8f0;
  background: #fdfefe;
}

.toolbar input {
  border: 1px solid #cfd7e3;
  border-radius: 8px;
  padding: 10px 12px;
  outline: none;
  background: #fff;
}

button {
  border: 1px solid #cfd7e3;
  border-radius: 8px;
  background: #fff;
  padding: 8px 14px;
  cursor: pointer;
}

button.primary {
  background: #2f67e6;
  border-color: #2f67e6;
  color: #fff;
}

button.danger {
  color: #b91c1c;
}

.item-area {
  overflow: auto;
  padding: 10px;
}

.item-list {
  list-style: none;
  margin: 0;
  padding: 0;
}

.item {
  display: grid;
  grid-template-columns: 34px 1fr auto;
  align-items: center;
  gap: 10px;
  background: #fff;
  border: 1px solid #e6ebf2;
  border-radius: 10px;
  padding: 9px 10px;
  margin-bottom: 7px;
  cursor: pointer;
}

.item:hover {
  border-color: #b6c9f1;
}

.item.active {
  border-color: #2f67e6;
  background: #edf3ff;
}

.item.separator {
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
}

.item-sub {
  font-size: 12px;
  color: #667085;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.badge {
  font-size: 12px;
  color: #455468;
}

.status-bar {
  border-top: 1px solid #dfe4ec;
  background: #fff;
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 12px;
  font-size: 13px;
}

.checkbox-inline {
  display: flex;
  align-items: center;
  gap: 6px;
}

.hotkey-wrap {
  display: flex;
  align-items: center;
  gap: 6px;
}

.hotkey-wrap input {
  width: 92px;
  padding: 5px 8px;
  border-radius: 6px;
  border: 1px solid #cfd7e3;
}

.status-text {
  color: #0f766e;
  min-width: 180px;
  text-align: right;
}

.status-text.error {
  color: #b91c1c;
}
</style>
