import React, { useRef } from 'react';
import { Menu } from 'antd';
import { Window } from '@tauri-apps/api/window';
import './TitleBar.css';

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

export const TitleBar: React.FC = () => {

  const win = Window.getCurrent();
  // 拖拽窗口
  const handleDrag = (e: React.MouseEvent) => {
    if (e.button === 0) {
      win.startDragging();
    }
  };

  // refs for each top menu button
  const menuRefs = useRef<(HTMLSpanElement | null)[]>([]);

  // 菜单弹出时动态设置弹出层left
  const handleOpenChange = (openKeys: string[]) => {
    if (openKeys.length > 0) {
      const key = openKeys[openKeys.length - 1];
      const idx = menuItems.findIndex(item => item.key === key);
      if (idx !== -1 && menuRefs.current[idx]) {
        const btn = menuRefs.current[idx]!;
        const left = btn.offsetLeft;
        setTimeout(() => {
          const popups = document.querySelectorAll('.ant-menu-submenu-popup');
          popups.forEach(popup => {
            (popup as HTMLElement).style.left = left + 'px';
          });
        }, 0);
      }
    }
  };

  // 生成 items 并加 ref（用 span）
  const itemsWithRef = menuItems.map((item, idx) => ({
    ...item,
    label: (
      <span ref={el => { menuRefs.current[idx] = el; }}>{item.label}</span>
    ),
  }));

  return (
    <div className="titlebar">
      <div className="titlebar-menu">
        <Menu
          mode="horizontal"
          items={itemsWithRef}
          onOpenChange={handleOpenChange}
        />
      </div>
      <div className="titlebar-drag" onMouseDown={handleDrag} data-tauri-drag-region></div>
      <div className="titlebar-actions">
        <button className="titlebar-btn" onClick={() => win.minimize()} title="最小化">─</button>
        <button className="titlebar-btn" onClick={() => win.toggleMaximize()} title="最大化/还原">☐</button>
        <button className="titlebar-btn close" onClick={() => win.close()} title="关闭">✕</button>
      </div>
    </div>
  );
};
