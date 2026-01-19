<template>
  <div class="api-test-container p-6">
    <h1 class="text-2xl font-bold mb-6">后端 API 测试</h1>
    
    <div class="grid grid-cols-1 md:grid-cols-2 gap-6">
      
      <!-- 基础功能测试 -->
      <el-card header="基础功能测试">
        <div class="space-y-4">
          <el-button type="primary" @click="testGetConfig" :loading="loading.getConfig">
            获取配置
          </el-button>
          
          <el-button type="success" @click="testAddItem" :loading="loading.addItem">
            添加测试程序
          </el-button>
          
          <el-button type="warning" @click="testLaunchNotepad" :loading="loading.launch">
            启动记事本
          </el-button>
          
          <el-button type="danger" @click="testLaunchAdmin" :loading="loading.launchAdmin">
            管理员启动 CMD
          </el-button>
        </div>
      </el-card>

      <!-- 高级功能测试 -->
      <el-card header="高级功能测试">
        <div class="space-y-4">
          <el-button type="info" @click="testGetIcon" :loading="loading.getIcon">
            获取记事本图标
          </el-button>
          
          <el-button type="primary" @click="testScanPrograms" :loading="loading.scan">
            扫描系统程序
          </el-button>
          
          <el-button @click="testUpdateUsage" :loading="loading.updateUsage">
            更新使用次数
          </el-button>
          
          <el-button type="danger" @click="testRemoveItem" :loading="loading.removeItem">
            删除测试程序
          </el-button>
        </div>
      </el-card>

      <!-- 测试结果显示 -->
      <el-card header="测试结果" class="md:col-span-2">
        <div class="test-results">
          <div v-for="(result, index) in testResults" :key="index" class="mb-4">
            <div class="flex items-center gap-2 mb-2">
              <el-tag :type="result.success ? 'success' : 'danger'">
                {{ result.success ? '✅' : '❌' }}
              </el-tag>
              <span class="font-semibold">{{ result.title }}</span>
              <span class="text-gray-500 text-sm">{{ result.timestamp }}</span>
            </div>
            <div class="bg-gray-100 p-2 rounded text-sm">
              <pre>{{ result.message }}</pre>
            </div>
          </div>
        </div>
      </el-card>

      <!-- 扫描结果展示 -->
      <el-card v-if="scannedPrograms.length > 0" header="扫描到的程序" class="md:col-span-2">
        <div class="grid grid-cols-1 md:grid-cols-3 gap-4">
          <div 
            v-for="program in scannedPrograms.slice(0, 12)" 
            :key="program.name"
            class="border p-3 rounded flex items-center gap-3"
          >
            <img 
              v-if="program.icon_base64" 
              :src="program.icon_base64" 
              alt="icon"
              class="w-8 h-8"
            />
            <div class="flex-1">
              <div class="font-medium text-sm">{{ program.name }}</div>
              <div class="text-xs text-gray-500 truncate">{{ program.target_path }}</div>
            </div>
          </div>
        </div>
        <div v-if="scannedPrograms.length > 12" class="mt-4 text-center text-gray-500">
          还有 {{ scannedPrograms.length - 12 }} 个程序...
        </div>
      </el-card>

    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive } from 'vue'
import { ElMessage } from 'element-plus'
import { invoke } from '@tauri-apps/api/core'

interface TestResult {
  title: string
  success: boolean
  message: string
  timestamp: string
}

interface LaunchItem {
  name: string
  target_path: string
  icon_location: string
  arguments: string
  count: number
  icon_base64?: string
}

const loading = reactive({
  getConfig: false,
  addItem: false,
  launch: false,
  launchAdmin: false,
  getIcon: false,
  scan: false,
  updateUsage: false,
  removeItem: false
})

const testResults = ref<TestResult[]>([])
const scannedPrograms = ref<LaunchItem[]>([])

const addTestResult = (title: string, success: boolean, message: string) => {
  testResults.value.unshift({
    title,
    success,
    message,
    timestamp: new Date().toLocaleTimeString()
  })
}

// 测试获取配置
const testGetConfig = async () => {
  loading.getConfig = true
  try {
    const config = await invoke('get_all_launch_items')
    addTestResult('获取配置', true, `配置获取成功:\n${JSON.stringify(config, null, 2)}`)
    ElMessage.success('配置获取成功')
  } catch (error) {
    addTestResult('获取配置', false, `错误: ${error}`)
    ElMessage.error(`获取配置失败: ${error}`)
  } finally {
    loading.getConfig = false
  }
}

// 测试添加程序
const testAddItem = async () => {
  loading.addItem = true
  try {
    const testItem: LaunchItem = {
      name: "记事本测试",
      target_path: "notepad.exe",
      icon_location: "",
      arguments: "",
      count: 0
    }

    await invoke('add_launch_item', {
      category: "测试分类",
      item: testItem
    })

    addTestResult('添加程序', true, `成功添加程序: ${testItem.name}`)
    ElMessage.success('程序添加成功')
  } catch (error) {
    addTestResult('添加程序', false, `错误: ${error}`)
    ElMessage.error(`添加程序失败: ${error}`)
  } finally {
    loading.addItem = false
  }
}

// 测试启动记事本
const testLaunchNotepad = async () => {
  loading.launch = true
  try {
    const request = {
      target_path: "notepad.exe",
      arguments: undefined,
      run_as_admin: false
    }

    await invoke('launch_program', { req: request })
    addTestResult('启动程序', true, '记事本启动成功')
    ElMessage.success('记事本启动成功')
  } catch (error) {
    addTestResult('启动程序', false, `错误: ${error}`)
    ElMessage.error(`启动失败: ${error}`)
  } finally {
    loading.launch = false
  }
}

// 测试管理员启动
const testLaunchAdmin = async () => {
  loading.launchAdmin = true
  try {
    const request = {
      target_path: "cmd.exe",
      arguments: undefined,
      run_as_admin: true
    }

    await invoke('launch_program', { req: request })
    addTestResult('管理员启动', true, 'CMD 以管理员权限启动成功')
    ElMessage.success('管理员启动成功')
  } catch (error) {
    addTestResult('管理员启动', false, `错误: ${error}`)
    ElMessage.error(`管理员启动失败: ${error}`)
  } finally {
    loading.launchAdmin = false
  }
}

// 测试获取图标
const testGetIcon = async () => {
  loading.getIcon = true
  try {
    const iconData = await invoke('get_icon_base64', { 
      path: "notepad.exe" 
    })
    
    if (typeof iconData === 'string' && iconData.includes('base64')) {
      addTestResult('获取图标', true, `图标提取成功，长度: ${iconData.length} 字符`)
      ElMessage.success('图标提取成功')
    } else {
      addTestResult('获取图标', false, '图标数据格式异常')
      ElMessage.warning('图标数据格式异常')
    }
  } catch (error) {
    addTestResult('获取图标', false, `错误: ${error}`)
    ElMessage.error(`图标提取失败: ${error}`)
  } finally {
    loading.getIcon = false
  }
}

// 测试扫描系统程序
const testScanPrograms = async () => {
  loading.scan = true
  try {
    ElMessage.info('正在扫描系统程序，请稍候...')
    const programs = await invoke('scan_system_programs') as LaunchItem[]
    
    scannedPrograms.value = programs
    addTestResult('扫描程序', true, `扫描完成，找到 ${programs.length} 个程序`)
    ElMessage.success(`扫描完成，找到 ${programs.length} 个程序`)
  } catch (error) {
    addTestResult('扫描程序', false, `错误: ${error}`)
    ElMessage.error(`扫描失败: ${error}`)
  } finally {
    loading.scan = false
  }
}

// 测试更新使用次数
const testUpdateUsage = async () => {
  loading.updateUsage = true
  try {
    await invoke('update_usage_count', {
      name: "记事本测试",
      category: "测试分类"
    })
    
    addTestResult('更新使用次数', true, '使用次数更新成功')
    ElMessage.success('使用次数更新成功')
  } catch (error) {
    addTestResult('更新使用次数', false, `错误: ${error}`)
    ElMessage.error(`更新失败: ${error}`)
  } finally {
    loading.updateUsage = false
  }
}

// 测试删除程序
const testRemoveItem = async () => {
  loading.removeItem = true
  try {
    await invoke('remove_launch_item', {
      name: "记事本测试",
      category: "测试分类"
    })
    
    addTestResult('删除程序', true, '测试程序删除成功')
    ElMessage.success('程序删除成功')
  } catch (error) {
    addTestResult('删除程序', false, `错误: ${error}`)
    ElMessage.error(`删除失败: ${error}`)
  } finally {
    loading.removeItem = false
  }
}
</script>

<style scoped>
.api-test-container {
  max-width: 1200px;
  margin: 0 auto;
}

.test-results {
  max-height: 400px;
  overflow-y: auto;
}

pre {
  white-space: pre-wrap;
  word-break: break-all;
}
</style>
