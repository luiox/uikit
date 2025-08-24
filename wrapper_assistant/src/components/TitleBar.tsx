import React from 'react';
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

  return (
    <div className="titlebar">
      <div className="titlebar-menu">
        <Menu mode="horizontal" items={menuItems} />
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
