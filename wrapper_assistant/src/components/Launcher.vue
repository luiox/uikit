<template>
  <div class="launcher">
    <div class="launcher-header">
      <a-typography-title :level="3" style="margin:0;padding:12px 24px;">Poner</a-typography-title>
    </div>
    <div class="launcher-body">
      <div class="menu">
        <a-menu :selectedKeys="[selectedCategory]" mode="inline" @click="handleMenuClick">
          <a-menu-item v-for="category in config?.categories || []" :key="category">{{ category }}</a-menu-item>
        </a-menu>
      </div>
      <div class="list">
        <a-list
          :header="selectedCategory"
          item-layout="horizontal"
          :data-source="getItemsByCategory(selectedCategory)"
        >
          <template #renderItem="{ item }">
            <a-list-item>
              <a-list-item-meta
                :avatar="getAvatar(item)"
                :title="item.name"
                :description="item.target_path"
              />
            </a-list-item>
          </template>
        </a-list>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue';
import { message, List as AList, Menu as AMenu, Typography as ATypography, Avatar as AAvatar, Spin as ASpin } from 'ant-design-vue';
import { fetchLaunchConfig } from '../api';
import './Launcher.less';

const config = ref<any>(null);
const selectedCategory = ref('');
const loading = ref(true);

onMounted(async () => {
  try {
    const cfg = await fetchLaunchConfig();
    config.value = cfg;
    selectedCategory.value = cfg.categories[0] || '';
  } catch {
    message.error('获取启动项配置失败');
  } finally {
    loading.value = false;
  }
});

function handleMenuClick({ key }: any) {
  selectedCategory.value = key;
}
function getItemsByCategory(category: string) {
  if (!config.value) return [];
  return config.value.items.filter((item: any) => item.category === category);
}
function getAvatar(item: any) {
  let avatarSrc: string | undefined = undefined;
  if (item.icon_base64 && item.icon_base64.startsWith('data:image')) {
    avatarSrc = item.icon_base64;
  } else if (item.icon_location && (item.icon_location.startsWith('http://') || item.icon_location.startsWith('https://'))) {
    avatarSrc = item.icon_location;
  }
  return avatarSrc ? <a-avatar src={avatarSrc}>{!avatarSrc && item.name[0]}</a-avatar> : <a-avatar>{item.name[0]}</a-avatar>;
}
</script>
