<template>
  <div class="editor-window" @contextmenu.prevent>
    <header class="editor-topbar" data-tauri-drag-region>
      <div class="editor-title">编辑启动项</div>
      <button class="close-btn no-drag" @click="onCancel">×</button>
    </header>

    <div class="editor-content">
      <label>名称</label>
      <input v-model="name" type="text" />

      <label>目标路径</label>
      <div class="path-row">
        <input v-model="targetPath" type="text" />
        <button class="path-btn" :disabled="itemType === 'separator'" @click="onPickTargetPath">浏览</button>
      </div>

      <label>图标路径</label>
      <input v-model="iconLocation" type="text" />

      <label>参数</label>
      <input v-model="argumentsText" type="text" />

      <label>类型</label>
      <select v-model="itemType">
        <option value="app">应用</option>
        <option value="separator">分隔项</option>
      </select>

      <div class="editor-actions">
        <button class="primary" @click="onSave">保存</button>
        <button @click="onCancel">取消</button>
      </div>

      <p class="status-text" :class="{ error: isError }">{{ statusText }}</p>
    </div>
  </div>
</template>

<script setup lang="ts">
import { onMounted, ref } from 'vue';
import { open } from '@tauri-apps/plugin-dialog';
import { closeCurrentWindow, closeEditor, getEditorContext, onEditorContextChanged, upsertItem } from './api';

const groupId = ref<string | null>(null);
const itemId = ref<string | null>(null);
const name = ref('');
const targetPath = ref('');
const iconLocation = ref('');
const argumentsText = ref('');
const itemType = ref<'app' | 'separator'>('app');
const statusText = ref('');
const isError = ref(false);

function deriveNameFromPath(pathValue: string): string {
  const normalized = pathValue.trim().replace(/[\\/]+$/, '');
  if (!normalized) return '';
  const base = normalized.split(/[/\\]/).pop() ?? '';
  if (!base) return '';
  return base.replace(/\.[^.]+$/, '');
}

function setStatus(message: string, error = false) {
  statusText.value = message;
  isError.value = error;
}

async function loadContext() {
  const ctx = await getEditorContext();
  if (!ctx) {
    setStatus('未获取到编辑上下文', true);
    return;
  }

  groupId.value = ctx.groupId;
  itemId.value = ctx.item?.id ?? null;
  name.value = ctx.item?.name ?? '';
  targetPath.value = ctx.item?.targetPath ?? '';
  iconLocation.value = ctx.item?.iconLocation ?? '';
  argumentsText.value = ctx.item?.arguments ?? '';
  itemType.value = (ctx.item?.itemType as 'app' | 'separator') ?? 'app';
  setStatus('');
}

async function onSave() {
  if (!groupId.value) {
    setStatus('无效分组', true);
    return;
  }

  const normalizedTargetPath = targetPath.value.trim();
  if (itemType.value === 'app' && !normalizedTargetPath) {
    setStatus('应用类型必须填写目标路径', true);
    return;
  }

  const n = name.value.trim();
  const resolvedName = n || (itemType.value === 'app' ? deriveNameFromPath(normalizedTargetPath) : '');
  if (!resolvedName) {
    setStatus('名称不能为空', true);
    return;
  }

  try {
    const normalizedIconPath = itemType.value === 'app'
      ? (iconLocation.value.trim() || normalizedTargetPath)
      : iconLocation.value.trim();

    await upsertItem(groupId.value, {
      id: itemId.value ?? undefined,
      itemType: itemType.value,
      name: resolvedName,
      targetPath: normalizedTargetPath,
      iconLocation: normalizedIconPath,
      arguments: argumentsText.value,
      enabled: true
    });
    await closeEditor();
  } catch (error) {
    setStatus(String(error), true);
  }
}

async function onCancel() {
  await closeCurrentWindow();
}

async function onPickTargetPath() {
  if (itemType.value === 'separator') {
    return;
  }

  try {
    const selected = await open({
      multiple: false,
      directory: false,
      filters: [
        { name: '可执行文件', extensions: ['exe'] },
        { name: '所有文件', extensions: ['*'] }
      ]
    });

    if (!selected || Array.isArray(selected)) {
      return;
    }

    targetPath.value = selected;
    if (!itemId.value && !name.value.trim()) {
      const guessed = deriveNameFromPath(selected);
      if (guessed) {
        name.value = guessed;
      }
    }
    if (!iconLocation.value.trim()) {
      iconLocation.value = selected;
    }
  } catch (error) {
    setStatus(`选择目标路径失败: ${String(error)}`, true);
  }
}

onMounted(async () => {
  await loadContext();
  await onEditorContextChanged(async () => {
    await loadContext();
  });
  document.addEventListener('keydown', (event) => {
    if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 's') {
      event.preventDefault();
      onSave();
    }
  });
});
</script>

<style scoped>
.editor-window {
  height: 100vh;
  display: grid;
  grid-template-rows: 40px 1fr;
  background: rgb(255, 255, 255);
  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif;
  overflow: hidden;
}

.editor-topbar {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 8px 0 12px;
  background: rgb(230, 230, 230);
  border-bottom: 1px solid rgb(210, 210, 210);
  user-select: none;
}

.editor-topbar[data-tauri-drag-region] {
  -webkit-app-region: drag;
}

.no-drag {
  -webkit-app-region: no-drag;
}

.editor-title {
  font-size: 14px;
  font-weight: 600;
}

.close-btn {
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

.close-btn:hover {
  background: #e81123;
  border-color: #e81123;
  color: #fff;
}

.editor-content {
  padding: 12px;
  display: grid;
  gap: 8px;
  align-content: start;
  overflow: auto;
  scrollbar-width: none;
  -ms-overflow-style: none;
}

.editor-content::-webkit-scrollbar {
  width: 0 !important;
  height: 0 !important;
  display: none;
}

.editor-content input,
.editor-content select {
  border: 1px solid #cfd7e3;
  border-radius: 7px;
  padding: 8px 10px;
  font-size: 13px;
}

.path-row {
  display: grid;
  grid-template-columns: 1fr auto;
  gap: 8px;
}

.path-btn {
  min-width: 66px;
}

.path-btn:disabled {
  cursor: not-allowed;
  opacity: 0.55;
}

.editor-actions {
  display: flex;
  justify-content: flex-end;
  gap: 8px;
  margin-top: 8px;
}

button {
  border: 1px solid #cfd7e3;
  border-radius: 7px;
  background: #fff;
  padding: 7px 14px;
  cursor: pointer;
}

button.primary {
  background: #2f67e6;
  border-color: #2f67e6;
  color: #fff;
}

.status-text {
  margin: 2px 0 0;
  min-height: 20px;
  text-align: right;
  color: #0f766e;
}

.status-text.error {
  color: #b91c1c;
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
