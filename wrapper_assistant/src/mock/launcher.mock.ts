import type { LaunchConfig } from '@/types/launcher';

// Mock的Base64图标数据 (简化版，实际中会是完整的图标数据)
const mockIconData = {
  notepad:
    'data:image/png;base64,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',
  calc: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAEaklEQVR4Ae3AA6AkWZbG8f937o3IzKdyS2Oubdu2bdu2bdu2bWmMnpZKr54yMyLu+Xa3anqmhztr1a++4Ru+oSUBYBtJ2OZ+krANgCRscz9JANjmfpIAsA2AJABsAyAJ2wiBIABscz/b3E8SAJIAASAJAEnY5nkYMAjxQJK4nySMsU3wTLYBcT9J2ObZDIBtJGGb+0lCEpcJjDHmgWwDYJsHoPJMksBgQBK2+ZVf+RX+I7zxG78x95OEbQCEqAAgABAIYZv/SLZ5FvMsxlRJ2MbmMiEkYZv7vdEbvREPJIn72eZ+ksBgjCR++Zd/mftJwjYIhHgmwjYAkpCEMc9NEvcTwja2sc39JAFgjCRs80C2kQTmMtsAVEnYxjaSkMQL8iu/8iu8KN74jd+YB5IE5jJjhACwTcVcJgnbAEhCiPvZ5t/LGAxCPAAVgRAAkrANgDH3k4Rt7vdGb/RGSOIyA+KyX/7lXwbANg9kG0nYBsA2AJKoPJNtJAFg8xxs80C/8iu/wgsjCds8kG0kAYC5whC2AZAE5pnMA0niX0sSDyQJ2wBIQhLGVBDPIsAgiecmiTd6ozdG4nkZjAGQhG2ekwCQBAYEtgEIMAC2sY0kbGObB7KNBLYBsA0G2yCexQZJgHg2Yxuby2wjCYAqCdtIAoNtnh9J/PIv/zIAb/RGb4QkfvmXfxmAN3qjN0ISv/zLvwzAG73RGyHE/SRhGwHGPADVNpIAQIBBEphnsY0kHsg295PEA0nigWwDYIwkAGwjiSoJANsASALAmAeyzRu/8Rtjm/u90Ru9EQAgJHijN3pjwABI4oEkYRsA20gCIGxjGwBJANg8FyEJ29xPEgCSkLhM4jJJPJAQACAAJGEbgMozScI2kgDzQBLYRggAY+5nm/tJQhK2kcT9jMFcZvMstqkAkrDNZeYySTy3X/6VXwbgjd/4jQH4lV/5FQDe6I3eCEn88i//MgBv9EZvxANJwjaSeCDbVB5AErYBsM39bJ6DbR5IEra5nyQwL5ANYCRRJQEgCQBJ2OY5GUm88Ru/MbYRwpg3eqM3AkASknijN3ojnh/bSMI2D2SbALCNbQBs89wk8UDGAEgCwDa2kcSziftJAvMskgCQRAUAIcA2z49tAH7lV34FgDd6ozcC4Jd/+ZcBeOM3emMQ/PIv/zIAb/zGb4zNcxJgLrPNMxG2kbhMEpL41zLmgWwD5oFsIwkASUjChsr9BBgQz0MSAG/8xm+Mbe73Rm/0RtzPNm/0Rm+EJCTx3CQBIAnbPBMBYBvbILANgCQeyDYYJAEgCQBJSEISkrANgM2z2GCb5yaJCiAJ29jmfra53y//8i/z72MkAWCb+9kQQtjmuUniP4okbJ6DJMDojd7ojQxgG0nYBkASALa5nyRsI4Qx95OEbe4nCQDbSMI2AEIYI4lnIgBsIwnbCAFgmweShG0AjAGQxANJQhK2uZ9tJAFgDIBtbGOb4LmJZzPPQxLPzTaSeG6SkIRtJCEEgCQkAfCPyuhM2d2E+M4AAAAASUVORK5CYII=',
};

export const mockLaunchConfig: LaunchConfig = {
  系统工具: [
    {
      name: '记事本',
      target_path: 'notepad.exe',
      icon_location: '',
      arguments: '',
      count: 15,
      icon_base64: mockIconData.notepad,
    },
    {
      name: '计算器',
      target_path: 'calc.exe',
      icon_location: '',
      arguments: '',
      count: 8,
      icon_base64: mockIconData.calc,
    },
    {
      name: '画图',
      target_path: 'mspaint.exe',
      icon_location: '',
      arguments: '',
      count: 3,
      icon_base64: undefined,
    },
    {
      name: '任务管理器',
      target_path: 'taskmgr.exe',
      icon_location: '',
      arguments: '',
      count: 25,
      icon_base64: undefined,
    },
  ],
  办公软件: [
    {
      name: 'Microsoft Word',
      target_path: 'C:\\Program Files\\Microsoft Office\\Office16\\WINWORD.EXE',
      icon_location: '',
      arguments: '',
      count: 12,
      icon_base64: undefined,
    },
    {
      name: 'Microsoft Excel',
      target_path: 'C:\\Program Files\\Microsoft Office\\Office16\\EXCEL.EXE',
      icon_location: '',
      arguments: '',
      count: 7,
      icon_base64: undefined,
    },
    {
      name: 'Microsoft PowerPoint',
      target_path: 'C:\\Program Files\\Microsoft Office\\Office16\\POWERPNT.EXE',
      icon_location: '',
      arguments: '',
      count: 4,
      icon_base64: undefined,
    },
  ],
  开发工具: [
    {
      name: 'Visual Studio Code',
      target_path: 'C:\\Users\\User\\AppData\\Local\\Programs\\Microsoft VS Code\\Code.exe',
      icon_location: '',
      arguments: '',
      count: 45,
      icon_base64: undefined,
    },
    {
      name: 'Git Bash',
      target_path: 'C:\\Program Files\\Git\\git-bash.exe',
      icon_location: '',
      arguments: '',
      count: 18,
      icon_base64: undefined,
    },
    {
      name: 'PowerShell',
      target_path: 'powershell.exe',
      icon_location: '',
      arguments: '-NoProfile',
      count: 32,
      icon_base64: undefined,
    },
    {
      name: '命令提示符',
      target_path: 'cmd.exe',
      icon_location: '',
      arguments: '',
      count: 9,
      icon_base64: undefined,
    },
  ],
  浏览器: [
    {
      name: 'Google Chrome',
      target_path: 'C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe',
      icon_location: '',
      arguments: '',
      count: 67,
      icon_base64: undefined,
    },
    {
      name: 'Microsoft Edge',
      target_path: 'C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe',
      icon_location: '',
      arguments: '',
      count: 23,
      icon_base64: undefined,
    },
    {
      name: 'Firefox',
      target_path: 'C:\\Program Files\\Mozilla Firefox\\firefox.exe',
      icon_location: '',
      arguments: '-private-window',
      count: 11,
      icon_base64: undefined,
    },
  ],
  媒体工具: [
    {
      name: 'VLC Media Player',
      target_path: 'C:\\Program Files\\VideoLAN\\VLC\\vlc.exe',
      icon_location: '',
      arguments: '',
      count: 6,
      icon_base64: undefined,
    },
    {
      name: 'Windows Media Player',
      target_path: 'wmplayer.exe',
      icon_location: '',
      arguments: '',
      count: 2,
      icon_base64: undefined,
    },
  ],
  游戏: [
    {
      name: '扫雷',
      target_path: 'winmine.exe',
      icon_location: '',
      arguments: '',
      count: 14,
      icon_base64: undefined,
    },
    {
      name: '纸牌',
      target_path: 'sol.exe',
      icon_location: '',
      arguments: '',
      count: 8,
      icon_base64: undefined,
    },
  ],
};

// Mock API函数
export const mockLauncherApi = {
  // 获取所有启动项配置
  async getAllLaunchItems(): Promise<LaunchConfig> {
    await new Promise(resolve => setTimeout(resolve, 500)); // 模拟网络延迟
    return JSON.parse(JSON.stringify(mockLaunchConfig)); // 深拷贝
  },

  // 启动程序
  async launchProgram(request: { target_path: string; arguments?: string; run_as_admin: boolean }): Promise<void> {
    await new Promise(resolve => setTimeout(resolve, 200));
    console.log(`Mock启动程序: ${request.target_path}`, request);

    // 模拟可能的错误
    if (request.target_path.includes('nonexistent')) {
      throw new Error('程序不存在');
    }
  },

  // 更新使用次数
  async updateUsageCount(name: string, category: string): Promise<void> {
    await new Promise(resolve => setTimeout(resolve, 100));
    console.log(`Mock更新使用次数: ${category} - ${name}`);

    // 更新mock数据中的计数
    if (mockLaunchConfig[category]) {
      const item = mockLaunchConfig[category].find(item => item.name === name);
      if (item) {
        item.count++;
      }
    }
  },

  // 添加启动项
  async addLaunchItem(category: string, item: any): Promise<void> {
    await new Promise(resolve => setTimeout(resolve, 300));
    console.log(`Mock添加程序: ${category}`, item);

    if (!mockLaunchConfig[category]) {
      mockLaunchConfig[category] = [];
    }

    // 检查是否已存在
    const exists = mockLaunchConfig[category].some(
      existingItem => existingItem.name === item.name || existingItem.target_path === item.target_path,
    );

    if (exists) {
      throw new Error('程序已存在');
    }

    mockLaunchConfig[category].push({
      ...item,
      count: 0,
      icon_base64: undefined,
    });
  },

  // 删除启动项
  async removeLaunchItem(name: string, category: string): Promise<void> {
    await new Promise(resolve => setTimeout(resolve, 200));
    console.log(`Mock删除程序: ${category} - ${name}`);

    if (mockLaunchConfig[category]) {
      const index = mockLaunchConfig[category].findIndex(item => item.name === name);
      if (index !== -1) {
        mockLaunchConfig[category].splice(index, 1);

        // 如果分类为空，删除分类
        if (mockLaunchConfig[category].length === 0) {
          delete mockLaunchConfig[category];
        }
      }
    }
  },

  // 编辑启动项
  async editLaunchItem(name: string, category: string, newItem: any): Promise<void> {
    await new Promise(resolve => setTimeout(resolve, 300));
    console.log(`Mock编辑程序: ${category} - ${name}`, newItem);

    if (mockLaunchConfig[category]) {
      const itemIndex = mockLaunchConfig[category].findIndex(item => item.name === name);
      if (itemIndex !== -1) {
        // 保持原有的使用次数和图标
        const originalCount = mockLaunchConfig[category][itemIndex].count;
        const originalIcon = mockLaunchConfig[category][itemIndex].icon_base64;

        mockLaunchConfig[category][itemIndex] = {
          ...newItem,
          count: originalCount,
          icon_base64: originalIcon,
        };
      }
    }
  },

  // 扫描系统程序
  async scanSystemPrograms(): Promise<any[]> {
    await new Promise(resolve => setTimeout(resolve, 2000)); // 模拟较长的扫描时间
    console.log('Mock扫描系统程序');

    // 返回一些模拟的系统程序
    return [
      {
        name: '注册表编辑器',
        target_path: 'regedit.exe',
        icon_location: '',
        arguments: '',
        count: 0,
        icon_base64: undefined,
      },
      {
        name: '系统配置',
        target_path: 'msconfig.exe',
        icon_location: '',
        arguments: '',
        count: 0,
        icon_base64: undefined,
      },
      {
        name: '磁盘清理',
        target_path: 'cleanmgr.exe',
        icon_location: '',
        arguments: '',
        count: 0,
        icon_base64: undefined,
      },
      {
        name: '磁盘管理',
        target_path: 'diskmgmt.msc',
        icon_location: '',
        arguments: '',
        count: 0,
        icon_base64: undefined,
      },
      {
        name: '设备管理器',
        target_path: 'devmgmt.msc',
        icon_location: '',
        arguments: '',
        count: 0,
        icon_base64: undefined,
      },
    ];
  },

  // 保存配置
  async saveConfig(config: LaunchConfig): Promise<void> {
    await new Promise(resolve => setTimeout(resolve, 200));
    console.log('Mock保存配置', config);
    // 实际项目中这里会保存到后端
  },
};
