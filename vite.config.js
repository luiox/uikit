import { defineConfig } from 'vite';
import { resolve } from 'path';
import vue from '@vitejs/plugin-vue';

export default defineConfig({
  plugins: [vue()],
  root: resolve(__dirname, 'ui'),
  build: {
    rollupOptions: {
      input: {
        index: resolve(__dirname, 'ui/index.html'),
        editor: resolve(__dirname, 'ui/editor.html')
      }
    },
    outDir: resolve(__dirname, 'ui/dist'),
    emptyOutDir: true
  }
});
