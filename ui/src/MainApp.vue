<template>
  <div class="app-window" @contextmenu.prevent>
    <header class="top-bar" data-tauri-drag-region>
      <div class="top-title">nassistant</div>
      <div class="top-actions">
        <button class="search-btn no-drag" :class="{ active: isSearchMode }" @click="toggleSearchMode" aria-label="搜索">
          <span class="search-icon" aria-hidden="true"></span>
        </button>
        <button class="hide-btn no-drag" @click="onHide">×</button>
      </div>
    </header>

    <div class="body-layout">
      <template v-if="!isSearchMode">
        <aside class="group-panel" @contextmenu.prevent="openGroupMenu">
          <ul class="group-list">
            <li
              v-for="group in groups"
              :key="group.id"
              class="group-item"
              :data-group-id="group.id"
              :class="{ active: group.id === activeGroupId }"
              @click="selectGroup(group.id)"
            >
              {{ group.name }}
            </li>
          </ul>
        </aside>

        <main class="item-panel" @contextmenu.prevent="openItemMenu">
          <ul class="item-list">
            <li
              v-for="item in filteredItems"
              :key="item.id"
              class="item-row"
              :data-item-id="item.id"
              :class="{ active: item.id === selectedItemId, separator: item.itemType === 'separator' }"
              @click="selectedItemId = item.id"
              @dblclick="item.itemType !== 'separator' && onLaunch()"
            >
              <span class="item-icon">
                <img v-if="item.itemType !== 'separator' && iconDataMap[item.id]" class="item-icon-image" :src="iconDataMap[item.id]" alt="" />
                <span v-else>{{ getFallbackIconText(item) }}</span>
              </span>
              <span class="item-name">{{ item.name || '(未命名)' }}</span>
              <span class="item-count">{{ item.launchCount ?? 0 }}</span>
            </li>
          </ul>
        </main>
      </template>

      <section v-else class="search-panel">
        <div class="search-input-wrap">
          <input
            ref="searchInputRef"
            v-model="searchKeyword"
            class="search-input"
            type="text"
            placeholder="搜索启动项"
          />
        </div>
        <ul class="item-list search-result-list">
          <li
            v-for="entry in searchedItems"
            :key="entry.item.id"
            class="item-row"
            :class="{ active: entry.item.id === selectedSearchItemId }"
            @click="selectedSearchItemId = entry.item.id"
            @dblclick="onLaunchSearchedItem(entry)"
          >
            <span class="item-icon">
              <img v-if="iconDataMap[entry.item.id]" class="item-icon-image" :src="iconDataMap[entry.item.id]" alt="" />
              <span v-else>{{ getFallbackIconText(entry.item) }}</span>
            </span>
            <span class="item-name">{{ entry.item.name || '(未命名)' }}</span>
            <span class="item-count">{{ entry.groupName }}</span>
          </li>
          <li v-if="searchedItems.length === 0" class="search-empty">没有匹配的启动项</li>
        </ul>
      </section>
    </div>

    <footer class="status-line" :class="{ error: isError }">{{ statusText }}</footer>

    <div
      v-if="groupMenu.visible"
      class="context-menu"
      :style="{ left: `${groupMenu.x}px`, top: `${groupMenu.y}px` }"
    >
      <button class="menu-item" @click="onGroupMenuAction">
        {{ groupMenuMode === 'rename' ? '重命名分组' : '添加分组' }}
      </button>
    </div>

    <div
      v-if="itemMenu.visible"
      class="context-menu"
      :style="{ left: `${itemMenu.x}px`, top: `${itemMenu.y}px` }"
    >
      <template v-if="itemMenuMode === 'item'">
        <button class="menu-item" @click="onItemMenuAction">编辑启动项</button>
        <div class="menu-submenu-wrap">
          <button class="menu-item">移动到</button>
          <div class="menu-submenu">
            <button
              v-for="group in moveTargetGroups"
              :key="group.id"
              class="menu-item"
              @click="onMoveItem(group.id)"
            >
              {{ group.name }}
            </button>
            <div v-if="moveTargetGroups.length === 0" class="menu-empty">没有其他分组</div>
          </div>
        </div>
      </template>
      <button v-else class="menu-item" @click="onItemMenuAction">添加启动项</button>
    </div>

    <div v-if="addGroupDialogVisible" class="dialog-mask" @click="closeAddGroupDialog">
      <div class="dialog-card" @click.stop>
        <div class="dialog-title">{{ addGroupDialogMode === 'rename' ? '重命名分组' : '添加分组' }}</div>
        <input
          ref="groupNameInputRef"
          v-model="newGroupName"
          class="dialog-input"
          type="text"
          :placeholder="addGroupDialogMode === 'rename' ? '输入新的分组名称' : '输入分组名称'"
          @keydown.enter.prevent="confirmAddGroup"
          @keydown.esc.prevent="closeAddGroupDialog"
        />
        <div class="dialog-actions">
          <button class="dialog-btn primary" @click="confirmAddGroup">确定</button>
          <button class="dialog-btn" @click="closeAddGroupDialog">取消</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed, nextTick, onMounted, ref } from 'vue';
import {
  addGroup,
  deleteItem,
  hideCurrentWindow,
  launchItem,
  loadLauncherState,
  moveItem,
  onDataChanged,
  onSettingsChanged,
  openEditor,
  renameGroup
} from './api';
import type { Group, LaunchItem } from './types';

interface SearchResultItem {
  groupId: string;
  groupName: string;
  item: LaunchItem;
}

const groups = ref<Group[]>([]);
const activeGroupId = ref<string | null>(null);
const selectedItemId = ref<string | null>(null);
const searchKeyword = ref('');
const isSearchMode = ref(false);
const selectedSearchItemId = ref<string | null>(null);
const statusText = ref('');
const isError = ref(false);
const groupMenu = ref({ visible: false, x: 0, y: 0 });
const groupMenuMode = ref<'add' | 'rename'>('add');
const groupMenuGroupId = ref<string | null>(null);
const itemMenu = ref({ visible: false, x: 0, y: 0 });
const itemMenuMode = ref<'add' | 'item'>('add');
const itemMenuItemId = ref<string | null>(null);
const iconDataMap = ref<Record<string, string>>({});
const addGroupDialogVisible = ref(false);
const newGroupName = ref('');
const addGroupDialogMode = ref<'add' | 'rename'>('add');
const addGroupDialogGroupId = ref<string | null>(null);
const groupNameInputRef = ref<HTMLInputElement | null>(null);
const searchInputRef = ref<HTMLInputElement | null>(null);

const activeGroup = computed(() => groups.value.find((g) => g.id === activeGroupId.value) ?? null);
const selectedItem = computed<LaunchItem | null>(() => activeGroup.value?.items.find((it) => it.id === selectedItemId.value) ?? null);

const filteredItems = computed(() => {
  return activeGroup.value?.items ?? [];
});

const searchedItems = computed<SearchResultItem[]>(() => {
  const key = searchKeyword.value.trim().toLowerCase();
  return groups.value.flatMap((group) => {
    return group.items
      .filter((item) => item.itemType !== 'separator')
      .filter((item) => {
        if (!key) {
          return true;
        }
        return (item.name ?? '').toLowerCase().includes(key);
      })
      .map((item) => ({
        groupId: group.id,
        groupName: group.name,
        item
      }));
  });
});

const moveTargetGroups = computed(() => {
  return groups.value.filter((group) => group.id !== activeGroupId.value);
});

function setStatus(message: string, error = false) {
  statusText.value = message;
  isError.value = error;
}

function selectGroup(id: string) {
  activeGroupId.value = id;
  selectedItemId.value = null;
}

async function toggleSearchMode() {
  isSearchMode.value = !isSearchMode.value;
  searchKeyword.value = '';
  selectedSearchItemId.value = null;
  closeMenus();
  if (isSearchMode.value) {
    await nextTick();
    searchInputRef.value?.focus();
  }
}

async function refresh() {
  const state = await loadLauncherState();
  const loadedGroups = state.groups ?? [];
  groups.value = loadedGroups;
  iconDataMap.value = state.itemIcons ?? {};

  const preferred = state.settings?.currentGroup;
  const byName = groups.value.find((g) => g.name === preferred);
  activeGroupId.value = byName?.id ?? groups.value[0]?.id ?? null;
  selectedItemId.value = null;
}

function getFallbackIconText(item: LaunchItem): string {
  if (item.itemType === 'separator') {
    return '--';
  }
  return item.name?.[0] || '?';
}

function closeMenus() {
  groupMenu.value.visible = false;
  itemMenu.value.visible = false;
}

function openGroupMenu(event: MouseEvent) {
  closeMenus();
  const target = event.target as HTMLElement | null;
  const groupItem = target?.closest('.group-item') as HTMLElement | null;
  const groupId = groupItem?.dataset.groupId ?? null;
  if (groupId) {
    groupMenuMode.value = 'rename';
    groupMenuGroupId.value = groupId;
  } else {
    groupMenuMode.value = 'add';
    groupMenuGroupId.value = null;
  }
  groupMenu.value = { visible: true, x: event.clientX, y: event.clientY };
}

function openItemMenu(event: MouseEvent) {
  closeMenus();
  const target = event.target as HTMLElement | null;
  const itemRow = target?.closest('.item-row') as HTMLElement | null;
  const itemId = itemRow?.dataset.itemId ?? null;
  if (itemId) {
    itemMenuMode.value = 'item';
    itemMenuItemId.value = itemId;
    selectedItemId.value = itemId;
  } else {
    itemMenuMode.value = 'add';
    itemMenuItemId.value = null;
  }
  itemMenu.value = { visible: true, x: event.clientX, y: event.clientY };
}

async function onAddGroup() {
  closeMenus();
  addGroupDialogMode.value = 'add';
  addGroupDialogGroupId.value = null;
  addGroupDialogVisible.value = true;
  newGroupName.value = '';
  await nextTick();
  groupNameInputRef.value?.focus();
}

async function onRenameGroup() {
  closeMenus();
  const groupId = groupMenuGroupId.value;
  if (!groupId) {
    return;
  }
  const group = groups.value.find((g) => g.id === groupId);
  if (!group) {
    return;
  }
  addGroupDialogMode.value = 'rename';
  addGroupDialogGroupId.value = groupId;
  addGroupDialogVisible.value = true;
  newGroupName.value = group.name;
  await nextTick();
  groupNameInputRef.value?.focus();
  groupNameInputRef.value?.select();
}

function closeAddGroupDialog() {
  addGroupDialogVisible.value = false;
  newGroupName.value = '';
}

async function confirmAddGroup() {
  const name = newGroupName.value.trim();
  if (!name) {
    setStatus('分组名称不能为空', true);
    return;
  }
  try {
    if (addGroupDialogMode.value === 'rename') {
      const targetId = addGroupDialogGroupId.value;
      if (!targetId) {
        return;
      }
      await renameGroup(targetId, name);
      setStatus('分组已重命名');
    } else {
      await addGroup(name);
      setStatus('分组已添加');
    }
    closeAddGroupDialog();
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

async function onEditItem() {
  closeMenus();
  if (!activeGroup.value || !itemMenuItemId.value) {
    return;
  }
  try {
    await openEditor(activeGroup.value.id, itemMenuItemId.value);
  } catch (error) {
    setStatus(`打开编辑窗口失败: ${String(error)}`, true);
  }
}

async function onMoveItem(targetGroupId: string) {
  closeMenus();
  if (!activeGroup.value || !itemMenuItemId.value) {
    return;
  }
  try {
    await moveItem(activeGroup.value.id, itemMenuItemId.value, targetGroupId);
    selectedItemId.value = null;
    setStatus('启动项已移动');
    await refresh();
  } catch (error) {
    setStatus(String(error), true);
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

async function onLaunchSearchedItem(entry: SearchResultItem) {
  try {
    const result = await launchItem(entry.groupId, entry.item.id);
    setStatus(result?.message ?? '启动成功');
    await refresh();
  } catch (error) {
    setStatus(String(error), true);
  }
}

async function onLaunchSelectedSearchedItem() {
  if (!selectedSearchItemId.value) {
    setStatus('请先选择要启动的条目', true);
    return;
  }
  const target = searchedItems.value.find((entry) => entry.item.id === selectedSearchItemId.value);
  if (!target) {
    setStatus('未找到对应启动项', true);
    return;
  }
  await onLaunchSearchedItem(target);
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
    if (event.key === 'Delete' && !isSearchMode.value) {
      onDelete();
    }
    if (event.key === 'Enter') {
      if (isSearchMode.value) {
        onLaunchSelectedSearchedItem();
      } else {
        onLaunch();
      }
    }
  });
  setStatus('就绪');
});

function onGroupMenuAction() {
  if (groupMenuMode.value === 'rename') {
    void onRenameGroup();
  } else {
    void onAddGroup();
  }
}

function onItemMenuAction() {
  if (itemMenuMode.value === 'item') {
    void onEditItem();
  } else {
    void onAddItem();
  }
}
</script>

<style scoped>
.app-window {
  width: 100%;
  height: 100vh;
  display: grid;
  grid-template-rows: 42px 1fr 28px;
  background: rgb(255, 255, 255);
  color: #1f2937;
  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif;
  overflow: hidden;
}

.top-bar {
  width: 100%;
  box-sizing: border-box;
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 8px 0 12px;
  background: rgb(230, 230, 230);
  border-bottom: 1px solid rgb(210, 210, 210);
  user-select: none;
}

.top-bar[data-tauri-drag-region] {
  -webkit-app-region: drag;
}

.no-drag {
  -webkit-app-region: no-drag;
}

.top-title {
  font-size: 14px;
  font-weight: 600;
}

.top-actions {
  display: flex;
  align-items: center;
  gap: 6px;
}

.search-btn,
.hide-btn {
  width: 30px;
  height: 28px;
  border: 1px solid rgb(200, 200, 200);
  border-radius: 6px;
  background: rgb(236, 236, 236);
  font-size: 18px;
  line-height: 26px;
  text-align: center;
  cursor: pointer;
  padding: 0;
}

.search-btn:hover {
  background: rgb(240, 240, 240);
}

.search-btn.active {
  background: rgb(236, 236, 236);
  border-color: rgb(180, 180, 180);
}

.search-icon {
  position: relative;
  display: inline-block;
  width: 12px;
  height: 12px;
  border: 1.6px solid #374151;
  border-radius: 50%;
  box-sizing: border-box;
}

.search-icon::after {
  content: '';
  position: absolute;
  width: 6px;
  height: 1.6px;
  background: #374151;
  right: -4px;
  bottom: -1px;
  transform: rotate(45deg);
  transform-origin: left center;
}

.hide-btn:hover {
  background: #e81123;
  border-color: #e81123;
  color: #fff;
}

.body-layout {
  display: grid;
  grid-template-columns: 220px 1fr;
  width: 100%;
  min-height: 0;
  min-width: 0;
  overflow: hidden;
}

.group-panel,
.item-panel {
  display: flex;
  flex-direction: column;
  min-height: 0;
  min-width: 0;
}

.group-panel {
  border-right: 1px solid rgb(210, 210, 210);
  background: rgb(230, 230, 230);
}

.item-panel {
  background: rgb(255, 255, 255);
}

.search-panel {
  display: flex;
  flex-direction: column;
  min-height: 0;
  min-width: 0;
  background: rgb(255, 255, 255);
  grid-column: 1 / -1;
}

.search-input-wrap {
  padding: 10px 12px;
  border-bottom: 1px solid rgb(210, 210, 210);
  box-sizing: border-box;
}

.search-input {
  width: 100%;
  box-sizing: border-box;
  border: 1px solid rgb(200, 200, 200);
  border-radius: 6px;
  padding: 8px 10px;
  font-size: 13px;
  background: #fff;
}

.search-result-list {
  padding: 10px;
}

.search-empty {
  list-style: none;
  padding: 10px;
  color: #64748b;
  font-size: 13px;
}

.section-title {
  padding: 10px 12px;
  border-bottom: 1px solid rgb(210, 210, 210);
  font-size: 13px;
  color: #495666;
  background: rgb(230, 230, 230);
}

.group-list,
.item-list {
  list-style: none;
  margin: 0;
  padding: 8px;
  overflow: auto;
  -ms-overflow-style: none;
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
  background: rgb(238, 238, 238);
  border-color: rgb(210, 210, 210);
}

.group-item.active {
  background: rgb(250, 250, 250);
  color: #1f2937;
  border-color: rgb(205, 205, 205);
}

.item-row {
  display: grid;
  grid-template-columns: 34px 1fr auto;
  align-items: center;
  gap: 10px;
  background: #fff;
  border: 1px solid transparent;
  border-radius: 8px;
  padding: 9px 10px;
  margin-bottom: 7px;
  cursor: pointer;
}

.item-row:hover {
  border-color: rgb(215, 215, 215);
  background: rgb(245, 245, 245);
}

.item-row.active {
  border-color: rgb(215, 215, 215);
  background: rgb(245, 245, 245);
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
  overflow: hidden;
}

.item-icon-image {
  width: 18px;
  height: 18px;
  object-fit: contain;
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
  border-top: 1px solid rgb(220, 220, 220);
  background: rgb(248, 248, 248);
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

.dialog-mask {
  position: fixed;
  inset: 0;
  background: rgba(32, 35, 42, 0.2);
  display: grid;
  place-items: center;
  z-index: 12000;
}

.dialog-card {
  width: 320px;
  background: rgb(246, 246, 246);
  border: 1px solid rgb(210, 210, 210);
  border-radius: 8px;
  box-shadow: 0 10px 26px rgba(15, 23, 42, 0.18);
  padding: 12px;
}

.dialog-title {
  font-size: 14px;
  font-weight: 600;
  margin-bottom: 10px;
}

.dialog-input {
  width: 100%;
  border: 1px solid rgb(200, 200, 200);
  border-radius: 6px;
  padding: 8px 10px;
  font-size: 13px;
  background: #fff;
}

.dialog-actions {
  margin-top: 10px;
  display: flex;
  justify-content: flex-end;
  gap: 8px;
}

.dialog-btn {
  border: 1px solid rgb(200, 200, 200);
  border-radius: 6px;
  background: #fff;
  padding: 6px 12px;
  font-size: 13px;
  cursor: pointer;
}

.dialog-btn.primary {
  background: rgb(0, 120, 215);
  border-color: rgb(0, 120, 215);
  color: #fff;
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

.menu-submenu-wrap {
  position: relative;
}

.menu-submenu {
  position: absolute;
  left: calc(100% + 6px);
  top: 0;
  min-width: 150px;
  background: #fff;
  border: 1px solid #d0dae8;
  border-radius: 8px;
  box-shadow: 0 10px 26px rgba(15, 23, 42, 0.16);
  padding: 6px;
  display: none;
}

.menu-submenu-wrap:hover .menu-submenu {
  display: block;
}

.menu-empty {
  padding: 8px 10px;
  font-size: 12px;
  color: #64748b;
}

.group-list,
.item-list {
  scrollbar-width: none;
}

.group-list::-webkit-scrollbar,
.item-list::-webkit-scrollbar {
  width: 0 !important;
  height: 0 !important;
  display: none;
}

:global(html),
:global(body),
:global(#app) {
  margin: 0;
  width: 100%;
  height: 100%;
  overflow: hidden;
  -ms-overflow-style: none;
  scrollbar-width: none;
}

:global(html::-webkit-scrollbar),
:global(body::-webkit-scrollbar),
:global(#app::-webkit-scrollbar) {
  width: 0 !important;
  height: 0 !important;
  display: none;
}
</style>
