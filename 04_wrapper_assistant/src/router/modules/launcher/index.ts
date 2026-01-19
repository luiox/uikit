import type { AppRouteRecordRaw } from '@/router/type';

const launcher: AppRouteRecordRaw[] = [
  {
    path: '/launcher',
    name: 'RtLauncher',
    component: () => import('@/views/launcher/index.vue'),
    meta: {
      title: '程序启动器',
      icon: 'iEL-management',
      position: 2,
    },
  },
];

export default launcher;
