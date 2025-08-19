// 定义启动项的接口
export interface LaunchItem {
  Name: string;
  TargetPath: string;
  IconLocation: string;
  Arguments: string;
  Count: number;
}

// 定义配置对象的类型
export interface Config {
  [category: string]: LaunchItem[];
}

// 配置对象
const config: Config = {
  "Embedded": [
    {
      "Name": "OpenMV IDE",
      "TargetPath": "D:\\Program Files\\OpenMV IDE\\bin\\openmvide.exe",
      "IconLocation": "D:\\Program Files\\OpenMV IDE\\bin\\openmvide.exe",
      "Arguments": "",
      "Count": 1
    },
    {
      "Name": "CanMV IDE K230",
      "TargetPath": "C:\\Program Files\\CanMV IDE K230\\bin\\canmvide.exe",
      "IconLocation": "C:\\Program Files\\CanMV IDE K230\\bin\\canmvide.exe",
      "Arguments": "",
      "Count": 0
    }
  ],
  "IDE": [
    {
        "Name": "qt6",
        "TargetPath": "%rr%\\Qt\\Tools\\QtCreator\\bin\\qtcreator.exe",
        "IconLocation": "%rr%\\Qt\\Tools\\QtCreator\\bin\\qtcreator.exe",
        "Arguments": "",
        "Count": 87
    },
    {
        "Name": "Visual Studio 2022",
        "TargetPath": "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\IDE\\devenv.exe",
        "IconLocation": "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\IDE\\devenv.exe",
        "Arguments": "",
        "Count": 432
    }
  ]
};

/**
 * 添加新的分类
 * @param category 分类名称
 */
export function addCategory(category: string): void {
  if (!config[category]) {
    config[category] = [];
  }
}

/**
 * 向指定分类添加启动项
 * @param category 分类名称
 * @param item 启动项
 */
export function addItemToCategory(category: string, item: LaunchItem): void {
  if (!config[category]) {
    addCategory(category);
  }
  config[category].push(item);
}

/**
 * 从指定分类中移除启动项
 * @param category 分类名称
 * @param itemName 启动项名称
 */
export function removeItemFromCategory(category: string, itemName: string): void {
  if (config[category]) {
    config[category] = config[category].filter(item => item.Name !== itemName);
  }
}

/**
 * 获取所有分类名称
 * @returns 分类名称数组
 */
export function getCategories(): string[] {
  return Object.keys(config);
}

/**
 * 获取配置对象
 * @returns 配置对象
 */
export function getConfig(): Config {
  return config;
}

export default config;