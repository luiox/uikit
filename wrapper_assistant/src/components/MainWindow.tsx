import React from 'react';
import { Menu } from 'antd';
import { GoldenLayoutComponent } from './GoldenLayoutComponent';
import 'antd/dist/reset.css';
import './MainWindow.css';

const items = [
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

export const MainWindow: React.FC = () => {
  return (
    <div className="main-window">
      <Menu mode="horizontal" items={items} />
      <div className="main-content">
        <GoldenLayoutComponent />
      </div>
    </div>
  );
};
