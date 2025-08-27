<template>
  <div class="launcher-container p-4">
    <!-- 开发模式提示 -->
    <el-alert
      v-if="USE_MOCK_DATA"
      title="🔧 开发模式 - 当前使用Mock数据"
      type="info"
      :closable="false"
      show-icon
      class="mb-4"
    >
      <template #default>
        <p class="mb-1">当前运行在开发模式下，所有操作都是模拟的，不会调用真实的后端API。</p>
        <p class="text-xs opacity-75">生产环境将自动切换到真实API模式。</p>
      </template>
    </el-alert>

    <!-- 顶部操作栏 -->
    <div class="toolbar mb-4">
      <el-row :gutter="16">
        <el-col :span="6">
          <el-select
            v-model="selectedCategory"
            placeholder="选择分类"
            size="large"
            style="width: 100%"
          >
            <el-option label="全部" value="all" />
            <el-option 
              v-for="category in categories"
              :key="category"
              :label="category"
              :value="category"
            />
          </el-select>
        </el-col>
        <el-col :span="10">
          <el-input
            v-model="searchKeyword"
            placeholder="搜索程序..."
            prefix-icon="Search"
            size="large"
            clearable
          />
        </el-col>
        <el-col :span="8">
          <div class="action-buttons">
            <el-button
              type="primary"
              size="large"
              @click="scanSystemPrograms"
              :loading="scanning"
            >
              扫描系统程序
            </el-button>
            <el-button
              type="success"
              size="large"
              @click="showAddCategoryDialog"
            >
              新建分组
            </el-button>
            <el-button
              type="warning"
              size="large"
              @click="showAddDialog"
            >
              新建启动项
            </el-button>
          </div>
        </el-col>
      </el-row>
    </div>

    <!-- 程序网格 -->
    <div class="programs-grid" v-loading="loading">
      <div 
        v-for="item in filteredPrograms"
        :key="`${item.category}-${item.name}`"
        class="program-card"
        @click="launchProgram(item, false)"
        @contextmenu.prevent="showContextMenu($event, item)"
      >
        <div class="program-icon">
          <img 
            v-if="item.icon_base64" 
            :src="item.icon_base64" 
            alt="icon"
            class="w-12 h-12 object-contain"
          />
          <el-icon v-else size="48px" color="#409EFF">
            <Document />
          </el-icon>
        </div>
        <div class="program-info">
          <p class="program-name" :title="item.name">{{ item.name }}</p>
          <p class="program-count">{{ item.count }} 次</p>
          <p class="program-path" :title="item.target_path">{{ shortenPath(item.target_path) }}</p>
        </div>
      </div>
    </div>

    <!-- 右键菜单 -->
    <div
      v-show="contextMenu.show"
      class="context-menu"
      :style="{ left: contextMenu.x + 'px', top: contextMenu.y + 'px' }"
      @click="contextMenu.show = false"
    >
      <div class="menu-item" @click="contextMenu.item && launchProgram(contextMenu.item, false)">
        <el-icon><VideoPlay /></el-icon>
        启动
      </div>
      <div class="menu-item" @click="contextMenu.item && launchProgram(contextMenu.item, true)">
        <el-icon><Key /></el-icon>
        管理员运行
      </div>
      <div class="menu-separator"></div>
      <div class="menu-item" @click="contextMenu.item && editProgram(contextMenu.item)">
        <el-icon><Edit /></el-icon>
        编辑
      </div>
      <div class="menu-item danger" @click="contextMenu.item && deleteProgram(contextMenu.item)">
        <el-icon><Delete /></el-icon>
        删除
      </div>
    </div>

    <!-- 添加程序对话框 -->
    <el-dialog
      v-model="addDialog.show"
      title="添加程序"
      width="600px"
      :before-close="handleAddDialogClose"
    >
      <el-form
        ref="addFormRef"
        :model="addDialog.form"
        :rules="addFormRules"
        label-width="100px"
      >
        <el-form-item label="程序名称" prop="name">
          <el-input v-model="addDialog.form.name" placeholder="请输入程序名称" />
        </el-form-item>
        <el-form-item label="分类" prop="category">
          <el-select
            v-model="addDialog.form.category"
            placeholder="选择或输入新分类"
            filterable
            allow-create
            style="width: 100%"
          >
            <el-option
              v-for="cat in categories"
              :key="cat"
              :label="cat"
              :value="cat"
            />
          </el-select>
        </el-form-item>
        <el-form-item label="程序路径" prop="target_path">
          <el-input
            v-model="addDialog.form.target_path"
            placeholder="请输入或选择程序路径"
          >
            <template #append>
              <el-button @click="selectProgramFile">浏览</el-button>
            </template>
          </el-input>
        </el-form-item>
        <el-form-item label="启动参数">
          <el-input
            v-model="addDialog.form.arguments"
            placeholder="启动程序时的命令行参数（可选）"
          />
        </el-form-item>
      </el-form>

      <template #footer>
        <el-button @click="addDialog.show = false">取消</el-button>
        <el-button type="primary" @click="confirmAddProgram" :loading="addDialog.loading">
          确定
        </el-button>
      </template>
    </el-dialog>

    <!-- 新建分组对话框 -->
    <el-dialog
      v-model="categoryDialog.show"
      title="新建分组"
      width="400px"
      :before-close="handleCategoryDialogClose"
    >
      <el-form
        ref="categoryFormRef"
        :model="categoryDialog.form"
        :rules="categoryFormRules"
        label-width="80px"
      >
        <el-form-item label="分组名称" prop="name">
          <el-input
            v-model="categoryDialog.form.name"
            placeholder="请输入分组名称"
            maxlength="20"
            show-word-limit
          />
        </el-form-item>
        <el-form-item label="描述">
          <el-input
            v-model="categoryDialog.form.description"
            type="textarea"
            placeholder="分组描述（可选）"
            :rows="3"
            maxlength="100"
            show-word-limit
          />
        </el-form-item>
      </el-form>

      <template #footer>
        <el-button @click="categoryDialog.show = false">取消</el-button>
        <el-button type="primary" @click="confirmAddCategory" :loading="categoryDialog.loading">
          确定
        </el-button>
      </template>
    </el-dialog>

    <!-- 统计信息 -->
    <div class="stats-bar">
      <span>共 {{ totalPrograms }} 个程序</span>
      <span v-if="searchKeyword">，筛选结果 {{ filteredPrograms.length }} 个</span>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, computed, onMounted, onUnmounted } from 'vue'
import { ElMessage, ElMessageBox, type FormInstance, type FormRules } from 'element-plus'
import { invoke } from '@tauri-apps/api/core'
import { open } from '@tauri-apps/plugin-dialog'
import { 
  Document, VideoPlay, Key, Edit, Delete 
} from '@element-plus/icons-vue'
// 导入mock数据和API
import { mockLauncherApi } from '@/mock/launcher.mock'

// 接口定义
interface LaunchItem {
  name: string
  target_path: string
  icon_location: string
  arguments: string
  count: number
  icon_base64?: string
}

interface LaunchProgramRequest {
  target_path: string
  arguments?: string
  run_as_admin: boolean
}

type LaunchConfig = Record<string, LaunchItem[]>

// 开发模式配置
const USE_MOCK_DATA = import.meta.env.VITE_USE_MOCK_DATA === 'true' || (import.meta.env.DEV && import.meta.env.VITE_USE_MOCK_DATA !== 'false')

// 响应式数据
const searchKeyword = ref('')
const selectedCategory = ref('all')
const launchConfig = ref<LaunchConfig>({})
const loading = ref(false)
const scanning = ref(false)

// 右键菜单
const contextMenu = reactive({
  show: false,
  x: 0,
  y: 0,
  item: null as (LaunchItem & { category: string }) | null
})

// 添加程序对话框
const addDialog = reactive({
  show: false,
  loading: false,
  form: {
    name: '',
    category: '',
    target_path: '',
    arguments: ''
  }
})

// 新建分组对话框
const categoryDialog = reactive({
  show: false,
  loading: false,
  form: {
    name: '',
    description: ''
  }
})

const addFormRef = ref<FormInstance>()
const categoryFormRef = ref<FormInstance>()
const addFormRules: FormRules = {
  name: [{ required: true, message: '请输入程序名称', trigger: 'blur' }],
  category: [{ required: true, message: '请选择分类', trigger: 'change' }],
  target_path: [{ required: true, message: '请输入程序路径', trigger: 'blur' }]
}

const categoryFormRules: FormRules = {
  name: [
    { required: true, message: '请输入分组名称', trigger: 'blur' },
    { min: 1, max: 20, message: '长度在 1 到 20 个字符', trigger: 'blur' }
  ]
}

// 计算属性
const categories = computed(() => Object.keys(launchConfig.value))

const allPrograms = computed(() => {
  const programs: Array<LaunchItem & { category: string }> = []
  for (const [category, items] of Object.entries(launchConfig.value)) {
    items.forEach(item => {
      programs.push({ ...item, category })
    })
  }
  return programs.sort((a, b) => b.count - a.count)
})

const filteredPrograms = computed(() => {
  let programs = allPrograms.value
  
  // 分类筛选
  if (selectedCategory.value !== 'all') {
    programs = programs.filter(p => p.category === selectedCategory.value)
  }
  
  // 搜索筛选
  if (searchKeyword.value) {
    const keyword = searchKeyword.value.toLowerCase()
    programs = programs.filter(p => 
      p.name.toLowerCase().includes(keyword) ||
      p.target_path.toLowerCase().includes(keyword)
    )
  }
  
  return programs
})

const totalPrograms = computed(() => allPrograms.value.length)

// 方法
const loadConfig = async () => {
  try {
    loading.value = true
    
    if (USE_MOCK_DATA) {
      // 开发环境使用mock数据
      console.log('🔧 开发模式：使用Mock数据')
      const config = await mockLauncherApi.getAllLaunchItems()
      launchConfig.value = config
      ElMessage.success('Mock数据加载成功')
    } else {
      // 生产环境使用真实API
      const config = await invoke('get_all_launch_items') as LaunchConfig
      launchConfig.value = config
    }
  } catch (error) {
    console.error('加载配置失败:', error)
    ElMessage.error('加载配置失败')
  } finally {
    loading.value = false
  }
}

const launchProgram = async (item: LaunchItem & { category: string }, runAsAdmin: boolean = false) => {
  try {
    if (USE_MOCK_DATA) {
      // Mock模式：模拟启动
      await mockLauncherApi.launchProgram({
        target_path: item.target_path,
        arguments: item.arguments || undefined,
        run_as_admin: runAsAdmin
      })
      
      // 更新使用次数
      await mockLauncherApi.updateUsageCount(item.name, item.category)
      
      // 重新加载配置
      await loadConfig()
      
      ElMessage.success(`Mock: ${runAsAdmin ? '以管理员权限' : ''}启动 ${item.name}`)
    } else {
      // 真实API调用
      const request: LaunchProgramRequest = {
        target_path: item.target_path,
        arguments: item.arguments || undefined,
        run_as_admin: runAsAdmin
      }
      
      await invoke('launch_program', { req: request })
      
      // 更新使用次数
      await invoke('update_usage_count', {
        name: item.name,
        category: item.category
      })
      
      // 重新加载配置以更新使用次数
      await loadConfig()
      
      ElMessage.success(`${runAsAdmin ? '以管理员权限' : ''}启动 ${item.name}`)
    }
  } catch (error) {
    console.error('启动失败:', error)
    ElMessage.error(`启动失败: ${error}`)
  }
  
  // 隐藏右键菜单
  contextMenu.show = false
}

const scanSystemPrograms = async () => {
  try {
    scanning.value = true
    ElMessage.info('正在扫描系统程序，请稍候...')
    
    if (USE_MOCK_DATA) {
      // Mock模式：模拟扫描
      const programs = await mockLauncherApi.scanSystemPrograms()
      
      if (programs.length === 0) {
        ElMessage.warning('未扫描到系统程序')
        return
      }
      
      // 添加到"系统程序"分类
      let addedCount = 0
      for (const program of programs) {
        try {
          await mockLauncherApi.addLaunchItem('系统程序', program)
          addedCount++
        } catch (error) {
          // 忽略重复添加的错误
          console.warn(`添加程序失败: ${program.name}`, error)
        }
      }
      
      await loadConfig()
      ElMessage.success(`Mock扫描完成，发现 ${programs.length} 个程序，新增 ${addedCount} 个`)
    } else {
      // 真实API调用
      const programs = await invoke('scan_system_programs') as LaunchItem[]
      
      if (programs.length === 0) {
        ElMessage.warning('未扫描到系统程序')
        return
      }
      
      // 添加到"系统程序"分类
      for (const program of programs) {
        try {
          await invoke('add_launch_item', {
            category: '系统程序',
            item: program
          })
        } catch (error) {
          // 忽略重复添加的错误
          console.warn(`添加程序失败: ${program.name}`, error)
        }
      }
      
      await loadConfig()
      ElMessage.success(`扫描完成，发现 ${programs.length} 个程序`)
    }
    
  } catch (error) {
    console.error('扫描失败:', error)
    ElMessage.error(`扫描失败: ${error}`)
  } finally {
    scanning.value = false
  }
}

const showContextMenu = (event: MouseEvent, item: LaunchItem & { category: string }) => {
  contextMenu.item = item
  contextMenu.x = event.clientX
  contextMenu.y = event.clientY
  contextMenu.show = true
}

const showAddDialog = () => {
  addDialog.form = {
    name: '',
    category: '',
    target_path: '',
    arguments: ''
  }
  addDialog.show = true
}

const showAddCategoryDialog = () => {
  console.log('点击新建分组按钮') // 调试信息
  categoryDialog.form = {
    name: '',
    description: ''
  }
  categoryDialog.show = true
  console.log('分组对话框应该显示:', categoryDialog.show) // 调试信息
}

const selectProgramFile = async () => {
  try {
    const selected = await open({
      filters: [
        {
          name: 'Executable',
          extensions: ['exe', 'bat', 'cmd']
        }
      ],
      multiple: false
    })
    
    if (selected && typeof selected === 'string') {
      addDialog.form.target_path = selected
      
      // 从路径中提取文件名作为默认程序名
      const fileName = selected.split('\\').pop()?.replace(/\.[^/.]+$/, '')
      if (fileName && !addDialog.form.name) {
        addDialog.form.name = fileName
      }
    }
  } catch (error) {
    console.error('选择文件失败:', error)
  }
}

const confirmAddProgram = async () => {
  if (!addFormRef.value) return
  
  try {
    await addFormRef.value.validate()
    
    addDialog.loading = true
    
    const item: LaunchItem = {
      name: addDialog.form.name,
      target_path: addDialog.form.target_path,
      icon_location: '',
      arguments: addDialog.form.arguments,
      count: 0,
      icon_base64: undefined
    }
    
    if (USE_MOCK_DATA) {
      // Mock模式：使用mock API
      await mockLauncherApi.addLaunchItem(addDialog.form.category, item)
      await loadConfig()
      ElMessage.success('Mock: 添加成功')
    } else {
      // 真实API调用
      await invoke('add_launch_item', {
        category: addDialog.form.category,
        item
      })
      
      await loadConfig()
      ElMessage.success('添加成功')
    }
    
    addDialog.show = false
    
  } catch (error) {
    console.error('添加失败:', error)
    ElMessage.error(`添加失败: ${error}`)
  } finally {
    addDialog.loading = false
  }
}

const editProgram = (_item: LaunchItem & { category: string }) => {
  // TODO: 实现编辑功能
  ElMessage.info('编辑功能待实现')
  contextMenu.show = false
}

const deleteProgram = async (item: LaunchItem & { category: string }) => {
  try {
    await ElMessageBox.confirm(`确定删除程序 "${item.name}" 吗？`, '确认删除', {
      type: 'warning'
    })
    
    if (USE_MOCK_DATA) {
      // Mock模式：使用mock API
      await mockLauncherApi.removeLaunchItem(item.name, item.category)
      await loadConfig()
      ElMessage.success('Mock: 删除成功')
    } else {
      // 真实API调用
      await invoke('remove_launch_item', {
        name: item.name,
        category: item.category
      })
      
      await loadConfig()
      ElMessage.success('删除成功')
    }
    
  } catch (error) {
    if (error !== 'cancel') {
      console.error('删除失败:', error)
      ElMessage.error('删除失败')
    }
  }
  
  contextMenu.show = false
}

const handleAddDialogClose = () => {
  addFormRef.value?.resetFields()
  addDialog.show = false
}

const handleCategoryDialogClose = () => {
  categoryFormRef.value?.resetFields()
  categoryDialog.show = false
}

const confirmAddCategory = async () => {
  if (!categoryFormRef.value) return
  
  try {
    await categoryFormRef.value.validate()
    
    categoryDialog.loading = true
    
    if (USE_MOCK_DATA) {
      // Mock模式：模拟创建分组
      await mockLauncherApi.createCategory(categoryDialog.form.name, categoryDialog.form.description || '')
      await loadConfig()
      ElMessage.success(`Mock: 分组 "${categoryDialog.form.name}" 创建成功`)
    } else {
      // 真实API调用
      await invoke('create_category', {
        name: categoryDialog.form.name,
        description: categoryDialog.form.description || ''
      })
      
      await loadConfig()
      ElMessage.success(`分组 "${categoryDialog.form.name}" 创建成功`)
    }
    
    categoryDialog.show = false
    
  } catch (error) {
    console.error('创建分组失败:', error)
    ElMessage.error(`创建分组失败: ${error}`)
  } finally {
    categoryDialog.loading = false
  }
}

const shortenPath = (path: string) => {
  if (path.length <= 50) return path
  return `...${path.slice(-47)}`
}

// 点击外部关闭右键菜单
const handleClickOutside = () => {
  contextMenu.show = false
}

// 生命周期
onMounted(() => {
  loadConfig()
  document.addEventListener('click', handleClickOutside)
})

onUnmounted(() => {
  document.removeEventListener('click', handleClickOutside)
})
</script>

<style scoped>
.launcher-container {
  height: 100vh;
  overflow-y: auto;
  position: relative;
}

.action-buttons {
  display: flex;
  gap: 8px;
}

.action-buttons .el-button {
  flex: 1;
  min-width: 0;
}

.programs-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(140px, 1fr));
  gap: 16px;
  padding: 16px 0;
  min-height: 400px;
}

.program-card {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 16px 12px;
  border-radius: 8px;
  border: 1px solid var(--el-border-color-light);
  cursor: pointer;
  transition: all 0.3s ease;
  background: var(--el-bg-color);
}

.program-card:hover {
  border-color: var(--el-color-primary);
  box-shadow: 0 4px 12px rgba(64, 158, 255, 0.2);
  transform: translateY(-2px);
}

.program-icon {
  margin-bottom: 12px;
}

.program-info {
  text-align: center;
  width: 100%;
}

.program-name {
  font-size: 14px;
  font-weight: 500;
  margin-bottom: 4px;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  color: var(--el-text-color-primary);
}

.program-count {
  font-size: 12px;
  color: var(--el-color-success);
  margin-bottom: 4px;
}

.program-path {
  font-size: 11px;
  color: var(--el-text-color-secondary);
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.context-menu {
  position: fixed;
  background: var(--el-bg-color);
  border: 1px solid var(--el-border-color);
  border-radius: 4px;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.15);
  z-index: 1000;
  min-width: 120px;
}

.menu-item {
  padding: 8px 12px;
  cursor: pointer;
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 14px;
  transition: background-color 0.2s;
}

.menu-item:hover {
  background: var(--el-fill-color-light);
}

.menu-item.danger {
  color: var(--el-color-danger);
}

.menu-separator {
  height: 1px;
  background: var(--el-border-color);
  margin: 4px 0;
}

.stats-bar {
  position: fixed;
  bottom: 0;
  left: 0;
  right: 0;
  background: var(--el-bg-color);
  border-top: 1px solid var(--el-border-color);
  padding: 8px 16px;
  font-size: 12px;
  color: var(--el-text-color-secondary);
}
</style>
