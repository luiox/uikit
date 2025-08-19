import React from 'react';
import { Menu, List, Typography, Avatar } from 'antd';
import config, { getCategories, LaunchItem } from '../config';
import './Launcher.less';

const { Title } = Typography;

const getItemsByCategory = (category: string): LaunchItem[] => {
  return config[category] || [];
};

const categories = getCategories();

const Launcher: React.FC = () => {
  const [selectedCategory, setSelectedCategory] = React.useState<string>(categories[0] || '');

  const handleMenuClick = (e: any) => {
    setSelectedCategory(e.key);
  };

  return (
    <div className="launcher">
      {/* 顶部标题栏 */}
      <div className="launcher-header">
        <Title level={3} style={{ margin: 0, padding: '12px 24px' }}>Poner</Title>
      </div>
      <div className="launcher-body">
        {/* 左侧分类菜单 */}
        <div className="menu">
          <Menu onClick={handleMenuClick} selectedKeys={[selectedCategory]} mode="inline">
            {categories.map((category) => (
              <Menu.Item key={category}>{category}</Menu.Item>
            ))}
          </Menu>
        </div>
        {/* 右侧启动项列表 */}
        <div className="list">
          <List
            header={<div style={{ fontWeight: 600, fontSize: 16 }}>{selectedCategory}</div>}
            itemLayout="horizontal"
            dataSource={getItemsByCategory(selectedCategory)}
            renderItem={(item: LaunchItem) => {
              // 只允许 http(s) 或 data:image 开头的路径作为 src，否则用默认头像
              let avatarSrc: string | undefined = undefined;
              if (item.IconLocation &&
                (item.IconLocation.startsWith('http://') ||
                 item.IconLocation.startsWith('https://') ||
                 item.IconLocation.startsWith('data:image'))
              ) {
                avatarSrc = item.IconLocation;
              }
              return (
                <List.Item>
                  <List.Item.Meta
                    avatar={<Avatar src={avatarSrc} >{!avatarSrc && item.Name[0]}</Avatar>}
                    title={item.Name}
                    description={item.TargetPath}
                  />
                </List.Item>
              );
            }}
          />
        </div>
      </div>
    </div>
  );
};

export default Launcher;