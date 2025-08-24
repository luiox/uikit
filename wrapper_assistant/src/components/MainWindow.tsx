import React from 'react';
import { GoldenLayoutComponent } from './GoldenLayoutComponent';
import { TitleBar } from './TitleBar';
import 'antd/dist/reset.css';
import './MainWindow.css';

export const MainWindow: React.FC = () => {
  return (
    <div className="main-window">
      <TitleBar />
      <div className="main-content">
        <GoldenLayoutComponent />
      </div>
    </div>
  );
};
