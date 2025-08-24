<template>
  <div class="main-window">
    <TitleBar />
    <div class="main-content">
      <GoldenLayoutComponent />
    </div>
  </div>
</template>

<script setup lang="ts">
import TitleBar from './TitleBar.vue';
import GoldenLayoutComponent from './GoldenLayoutComponent.vue';
import './MainWindow.css';
</script>
