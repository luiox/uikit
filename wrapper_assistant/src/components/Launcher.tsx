import React, { useEffect, useState } from 'react';
import { Menu, List, Typography, Avatar, Spin, message } from 'antd';
import { fetchLaunchConfig, LaunchConfig, LaunchItem } from '../api';
import './Launcher.less';

const { Title } = Typography;

const Launcher: React.FC = () => {
  const [config, setConfig] = useState<LaunchConfig | null>(null);
  const [selectedCategory, setSelectedCategory] = useState<string>('');
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    fetchLaunchConfig()
      .then(cfg => {
        setConfig(cfg);
        setSelectedCategory(cfg.categories[0] || '');
      })
      .catch(() => {
        message.error('获取启动项配置失败');
      })
      .finally(() => setLoading(false));
  }, []);

  const handleMenuClick = (e: any) => {
    setSelectedCategory(e.key);
  };

  const getItemsByCategory = (category: string): LaunchItem[] => {
    if (!config) return [];
    return config.items.filter(item => item.category === category);
  };

  if (loading) {
    return <div style={{textAlign: 'center', marginTop: 100}}><Spin size="large" /></div>;
  }

  if (!config) {
    return <div style={{textAlign: 'center', marginTop: 100}}>配置加载失败</div>;
  }

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
            {config.categories.map((category) => (
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
              let avatarSrc: string | undefined = undefined;
              if (item.icon_base64 && item.icon_base64.startsWith('data:image')) {
                avatarSrc = item.icon_base64;
              } else if (item.icon_location && (item.icon_location.startsWith('http://') || item.icon_location.startsWith('https://'))) {
                avatarSrc = item.icon_location;
              }
              return (
                <List.Item>
                  <List.Item.Meta
                    avatar={<Avatar src={avatarSrc}>{!avatarSrc && item.name[0]}</Avatar>}
                    title={item.name}
                    description={item.target_path}
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