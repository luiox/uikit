import React from 'react';
import { Menu, List, Typography } from 'antd';
import config, { getCategories, LaunchItem } from '../config';

const { Title } = Typography;

// 根据当前选中的分类获取启动项列表
const getItemsByCategory = (category: string): LaunchItem[] => {
  return config[category] || [];
};

// 获取所有分类名称
const categories = getCategories();

const Launcher: React.FC = () => {
  const [selectedCategory, setSelectedCategory] = React.useState<string>('Embedded');

  // 当选择不同的分类时更新状态
  const handleMenuClick = (e: any) => {
    setSelectedCategory(e.key);
  };

  return (
    <div>
      <Title level={2}>Poner</Title>
      {/* 分类菜单 */}
      <Menu onClick={handleMenuClick} selectedKeys={[selectedCategory]} mode="inline">
        {categories.map((category) => (
          <Menu.Item key={category}>{category}</Menu.Item>
        ))}
      </Menu>

      {/* 启动项列表 */}
      <List
        itemLayout="horizontal"
        dataSource={getItemsByCategory(selectedCategory)}
        renderItem={(item: LaunchItem) => (
          <List.Item>
            <List.Item.Meta title={item.Name} description={item.TargetPath} />
          </List.Item>
        )}
      />
    </div>
  );
};

export default Launcher;