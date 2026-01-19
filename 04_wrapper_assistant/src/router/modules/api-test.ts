export default {
  path: '/api-test',
  name: 'ApiTest',
  component: () => import('@/views/api-test.vue'),
  meta: {
    title: 'API测试',
    icon: 'TestTube',
    showLink: true,
    rank: 99,
  },
};
