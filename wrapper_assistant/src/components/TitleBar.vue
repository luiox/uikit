<template>
  <div class="titlebar">
    <div class="titlebar-menu">
      <a-menu mode="horizontal" @openChange="handleOpenChange">
        <a-sub-menu v-for="(item, idx) in menuItems" :key="item.key">
          <template #title>
            <span ref="setMenuRef(idx)">{{ item.label }}</span>
          </template>
          <a-menu-item v-for="child in item.children" :key="child.key">{{ child.label }}</a-menu-item>
        </a-sub-menu>
      </a-menu>
    </div>
    <div class="titlebar-drag" @mousedown="handleDrag" data-tauri-drag-region></div>
    <div class="titlebar-actions">
      <button class="titlebar-btn" @click="minimize" title="最小化">─</button>
      <button class="titlebar-btn" @click="toggleMaximize" title="最大化/还原">▢</button>
      <button class="titlebar-btn close" @click="close" title="关闭">✕</button>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue';
import { Window } from '@tauri-apps/api/window';
import './TitleBar.css';

const win = Window.getCurrent();
const menuItems = [
  {
    label: '文件',
    key: 'file',
    children: [
      { label: '新建', key: 'new' },
      { label: '打开', key: 'open' },
      { label: '保存', key: 'save' },
    ],
  },
  {
    label: '编辑',
    key: 'edit',
    children: [
      { label: '撤销', key: 'undo' },
      { label: '重做', key: 'redo' },
    ],
  },
  {
    label: '帮助',
    key: 'help',
    children: [
      { label: '关于', key: 'about' },
    ],
  },
];

const menuRefs = ref<(HTMLElement | null)[]>([]);
function setMenuRef(idx: number) {
  return (el: HTMLElement | null) => {
    menuRefs.value[idx] = el;
  };
}

function handleOpenChange(openKeys: string[]) {
  if (openKeys.length > 0) {
    const key = openKeys[openKeys.length - 1];
    const idx = menuItems.findIndex(item => item.key === key);
    if (idx !== -1 && menuRefs.value[idx]) {
      const btn = menuRefs.value[idx]!;
      const left = btn.offsetLeft;
      setTimeout(() => {
        const popups = document.querySelectorAll('.ant-menu-submenu-popup');
        popups.forEach(popup => {
          (popup as HTMLElement).style.left = left + 'px';
        });
      }, 0);
    }
  }
}
function handleDrag(e: MouseEvent) {
  if (e.button === 0) {
    win.startDragging();
  }
}
function minimize() {
  win.minimize();
}
function toggleMaximize() {
  win.toggleMaximize();
}
function close() {
  win.close();
}
</script>
