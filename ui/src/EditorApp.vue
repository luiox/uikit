<template>
  <div class="editor-body" @contextmenu.prevent>
    <div class="editor-card">
      <h2>编辑启动项</h2>

      <label>名称</label>
      <input v-model="name" type="text" />

      <label>目标路径</label>
      <input v-model="targetPath" type="text" />

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
import { closeEditor, getEditorContext, onEditorContextChanged, upsertItem } from './api';

const groupId = ref<string | null>(null);
const itemId = ref<string | null>(null);
const name = ref('');
const targetPath = ref('');
const iconLocation = ref('');
const argumentsText = ref('');
const itemType = ref<'app' | 'separator'>('app');
const statusText = ref('');
const isError = ref(false);

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

  const n = name.value.trim();
  if (!n) {
    setStatus('名称不能为空', true);
    return;
  }

  if (itemType.value === 'app' && !targetPath.value.trim()) {
    setStatus('应用类型必须填写目标路径', true);
    return;
  }

  try {
    await upsertItem(groupId.value, {
      id: itemId.value ?? undefined,
      itemType: itemType.value,
      name: n,
      targetPath: targetPath.value,
      iconLocation: iconLocation.value,
      arguments: argumentsText.value,
      enabled: true
    });
    await closeEditor();
  } catch (error) {
    setStatus(String(error), true);
  }
}

async function onCancel() {
  await closeEditor();
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
.editor-body {
  margin: 0;
  background: #eef2f8;
  display: grid;
  place-items: center;
  height: 100vh;
  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif;
}

.editor-card {
  width: min(560px, calc(100vw - 24px));
  background: #fff;
  border: 1px solid #dde5f0;
  border-radius: 12px;
  padding: 16px;
  display: grid;
  gap: 8px;
}

.editor-card h2 {
  margin: 2px 0 8px;
  font-size: 20px;
}

.editor-card input,
.editor-card select {
  border: 1px solid #cfd7e3;
  border-radius: 8px;
  padding: 9px 10px;
  font-size: 14px;
}

.editor-actions {
  display: flex;
  justify-content: flex-end;
  gap: 8px;
  margin-top: 8px;
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

.status-text {
  margin: 2px 0 0;
  min-height: 20px;
  text-align: right;
  color: #0f766e;
}

.status-text.error {
  color: #b91c1c;
}
</style>
