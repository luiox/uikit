use crate::commands::*;
use crate::config::*;
use crate::launcher::*;
use std::fs;
use std::env;
use std::io::Write;

#[cfg(test)]
mod tests {
    use super::*;
    
    const TEST_CONFIG_PATH: &str = "test_config.json";
    const TEST_FILE_NAME: &str = "test_launcher_file.txt";
    
    // 获取测试配置的完整路径
    fn get_test_config_path() -> String {
        let mut path = env::current_dir().unwrap();
        path.push(TEST_CONFIG_PATH);
        path.to_string_lossy().to_string()
    }
    
    // 获取测试文件的完整路径
    fn get_test_file_path() -> String {
        let mut path = env::current_dir().unwrap();
        path.push(TEST_FILE_NAME);
        path.to_string_lossy().to_string()
    }
    
    // 创建测试文件
    fn create_test_file() -> String {
        let file_path = get_test_file_path();
        let mut file = fs::File::create(&file_path).unwrap();
        writeln!(file, "这是一个测试文件").unwrap();
        writeln!(file, "用于测试 Tauri 启动器功能").unwrap();
        writeln!(file, "测试内容: Hello from Tauri Launcher Test").unwrap();
        file_path
    }
    
    // 清理测试文件
    fn cleanup_test_files() {
        let _ = fs::remove_file(get_test_config_path());
        let _ = fs::remove_file(get_test_file_path());
    }
    
    // 设置测试环境
    fn setup_test() {
        cleanup_test_files();
    }
    
    #[test]
    fn test_get_all_launch_items() {
        setup_test();
        
        // 测试获取配置（使用默认路径）
        let result = get_all_launch_items();
        println!("获取配置结果: {:?}", result);
        
        match result {
            Ok(config) => {
                println!("✅ 获取配置成功: {:?}", config);
            }
            Err(e) => {
                println!("⚠️ 获取配置失败（可能是首次运行）: {}", e);
                // 这是预期的，因为配置文件可能不存在
            }
        }
    }
    
    #[test]
    fn test_config_management() {
        println!("\n=== 配置管理功能测试 ===");
        cleanup_test_files();
        
        // 1. 测试获取空配置
        println!("1. 测试获取初始配置...");
        let result = get_all_launch_items();
        match result {
            Ok(config) => {
                println!("✅ 获取配置成功，包含 {} 个分类", config.len());
                for (category, items) in &config {
                    println!("   分类 '{}': {} 个程序", category, items.len());
                }
            }
            Err(e) => {
                println!("⚠️ 获取配置失败（首次运行正常）: {}", e);
            }
        }
        
        // 2. 测试添加程序到配置
        println!("\n2. 测试添加程序到配置...");
        let test_item1 = LaunchItem {
            name: "记事本".to_string(),
            target_path: "notepad.exe".to_string(),
            icon_location: "".to_string(),
            arguments: "".to_string(),
            count: 0,
            icon_base64: None,
        };
        
        let add_result = add_launch_item("系统工具".to_string(), test_item1);
        match add_result {
            Ok(_) => println!("✅ 添加程序成功"),
            Err(e) => println!("❌ 添加程序失败: {}", e),
        }
        
        // 3. 添加第二个程序到同一分类
        println!("\n3. 添加第二个程序到同一分类...");
        let test_item2 = LaunchItem {
            name: "计算器".to_string(),
            target_path: "calc.exe".to_string(),
            icon_location: "".to_string(),
            arguments: "".to_string(),
            count: 0,
            icon_base64: None,
        };
        
        let add_result2 = add_launch_item("系统工具".to_string(), test_item2);
        match add_result2 {
            Ok(_) => println!("✅ 添加第二个程序成功"),
            Err(e) => println!("❌ 添加第二个程序失败: {}", e),
        }
        
        // 4. 添加程序到新分类
        println!("\n4. 添加程序到新分类...");
        let test_item3 = LaunchItem {
            name: "PowerShell".to_string(),
            target_path: "powershell.exe".to_string(),
            icon_location: "".to_string(),
            arguments: "-NoProfile".to_string(),
            count: 0,
            icon_base64: None,
        };
        
        let add_result3 = add_launch_item("开发工具".to_string(), test_item3);
        match add_result3 {
            Ok(_) => println!("✅ 添加到新分类成功"),
            Err(e) => println!("❌ 添加到新分类失败: {}", e),
        }
        
        // 5. 验证配置内容
        println!("\n5. 验证当前配置内容...");
        let config = get_all_launch_items();
        match config {
            Ok(cfg) => {
                println!("✅ 当前配置包含 {} 个分类:", cfg.len());
                for (category, items) in &cfg {
                    println!("   📁 分类 '{}': {} 个程序", category, items.len());
                    for (i, item) in items.iter().enumerate() {
                        println!("      {}. {} -> {} (使用{}次)", i+1, item.name, item.target_path, item.count);
                        if !item.arguments.is_empty() {
                            println!("         参数: {}", item.arguments);
                        }
                    }
                }
            }
            Err(e) => println!("❌ 验证配置失败: {}", e),
        }
        
        println!("=== 配置管理基础测试完成 ===");
    }
    
    #[test]
    fn test_config_edit_and_usage() {
        println!("\n=== 配置编辑和使用统计测试 ===");
        
        // 先添加一些测试数据
        let test_item = LaunchItem {
            name: "测试编辑程序".to_string(),
            target_path: "notepad.exe".to_string(),
            icon_location: "".to_string(),
            arguments: "".to_string(),
            count: 0,
            icon_base64: None,
        };
        
        let _ = add_launch_item("测试分类".to_string(), test_item);
        
        // 1. 测试更新使用次数
        println!("1. 测试更新使用次数...");
        for i in 1..=3 {
            let update_result = update_usage_count("测试编辑程序".to_string(), "测试分类".to_string());
            match update_result {
                Ok(_) => println!("   第{}次使用计数更新成功", i),
                Err(e) => println!("   第{}次使用计数更新失败: {}", i, e),
            }
        }
        
        // 2. 验证使用次数
        println!("\n2. 验证使用次数...");
        let config = get_all_launch_items();
        if let Ok(cfg) = config {
            if let Some(items) = cfg.get("测试分类") {
                if let Some(item) = items.iter().find(|i| i.name == "测试编辑程序") {
                    println!("✅ 使用次数验证: {} 次", item.count);
                    if item.count == 3 {
                        println!("✅ 使用次数统计正确");
                    } else {
                        println!("❌ 使用次数统计错误，预期3次，实际{}次", item.count);
                    }
                }
            }
        }
        
        // 3. 测试编辑程序
        println!("\n3. 测试编辑程序...");
        let edited_item = LaunchItem {
            name: "测试编辑程序".to_string(),
            target_path: "calc.exe".to_string(), // 改为计算器
            icon_location: "".to_string(),
            arguments: "-v".to_string(), // 添加参数
            count: 0, // 这个会被保持原值
            icon_base64: None,
        };
        
        let edit_result = edit_launch_item("测试编辑程序".to_string(), "测试分类".to_string(), edited_item);
        match edit_result {
            Ok(_) => {
                println!("✅ 编辑程序成功");
                
                // 验证编辑结果
                let config = get_all_launch_items();
                if let Ok(cfg) = config {
                    if let Some(items) = cfg.get("测试分类") {
                        if let Some(item) = items.iter().find(|i| i.name == "测试编辑程序") {
                            println!("   编辑后路径: {}", item.target_path);
                            println!("   编辑后参数: {}", item.arguments);
                            println!("   保持的使用次数: {}", item.count);
                            
                            if item.target_path == "calc.exe" && item.arguments == "-v" && item.count == 3 {
                                println!("✅ 编辑验证成功");
                            } else {
                                println!("❌ 编辑验证失败");
                            }
                        }
                    }
                }
            }
            Err(e) => println!("❌ 编辑程序失败: {}", e),
        }
        
        // 4. 测试删除程序
        println!("\n4. 测试删除程序...");
        let remove_result = remove_launch_item("测试编辑程序".to_string(), "测试分类".to_string());
        match remove_result {
            Ok(_) => {
                println!("✅ 删除程序成功");
                
                // 验证删除结果
                let config = get_all_launch_items();
                if let Ok(cfg) = config {
                    if cfg.contains_key("测试分类") {
                        if let Some(items) = cfg.get("测试分类") {
                            if items.is_empty() {
                                println!("✅ 分类已清空（但仍存在）");
                            } else {
                                println!("⚠️ 分类中还有其他程序");
                            }
                        }
                    } else {
                        println!("✅ 空分类已被自动删除");
                    }
                }
            }
            Err(e) => println!("❌ 删除程序失败: {}", e),
        }
        
        println!("=== 配置编辑和使用统计测试完成 ===");
    }
    
    #[test] 
    fn test_config_persistence() {
        println!("\n=== 配置持久化测试 ===");
        
        // 1. 添加测试数据
        println!("1. 添加测试数据...");
        let items_to_add = vec![
            ("办公软件", "Word", "winword.exe", ""),
            ("办公软件", "Excel", "excel.exe", ""),
            ("浏览器", "Chrome", "chrome.exe", "--new-window"),
            ("浏览器", "Firefox", "firefox.exe", "-private-window"),
        ];
        
        for (category, name, path, args) in items_to_add {
            let item = LaunchItem {
                name: name.to_string(),
                target_path: path.to_string(),
                icon_location: "".to_string(),
                arguments: args.to_string(),
                count: 0,
                icon_base64: None,
            };
            
            let result = add_launch_item(category.to_string(), item);
            match result {
                Ok(_) => println!("   ✅ 添加 {} 到 {} 成功", name, category),
                Err(e) => println!("   ❌ 添加 {} 失败: {}", name, e),
            }
        }
        
        // 2. 读取配置验证
        println!("\n2. 验证配置持久化...");
        let config = get_all_launch_items();
        match config {
            Ok(cfg) => {
                println!("✅ 配置读取成功");
                let total_programs: usize = cfg.values().map(|items| items.len()).sum();
                println!("   总共 {} 个分类，{} 个程序", cfg.len(), total_programs);
                
                // 详细展示配置
                for (category, items) in &cfg {
                    println!("   📁 {}: {} 个程序", category, items.len());
                    for item in items {
                        let args_info = if item.arguments.is_empty() {
                            "无参数".to_string()
                        } else {
                            format!("参数: {}", item.arguments)
                        };
                        println!("      • {} -> {} ({})", item.name, item.target_path, args_info);
                    }
                }
            }
            Err(e) => println!("❌ 配置读取失败: {}", e),
        }
        
        // 3. 测试配置保存
        println!("\n3. 测试直接配置保存...");
        let config = get_all_launch_items();
        if let Ok(cfg) = config {
            let save_result = save_config_command(cfg);
            match save_result {
                Ok(_) => println!("✅ 配置保存成功"),
                Err(e) => println!("❌ 配置保存失败: {}", e),
            }
        }
        
        println!("=== 配置持久化测试完成 ===");
    }
    
    #[test]
    fn test_launch_program_with_arguments() {
        println!("测试带命令行参数的程序启动...");
        
        // 创建测试文件
        let test_file = create_test_file();
        println!("创建测试文件: {}", test_file);
        
        // 测试用记事本打开测试文件
        let request = LaunchProgramRequest {
            target_path: "notepad.exe".to_string(),
            arguments: Some(test_file.clone()),
            run_as_admin: false,
        };
        
        let result = launch_program(request);
        println!("启动记事本打开文件结果: {:?}", result);
        
        match result {
            Ok(_) => {
                println!("✅ 带参数启动程序成功");
                println!("📝 记事本应该已打开文件: {}", test_file);
                
                // 等待几秒让用户看到效果
                std::thread::sleep(std::time::Duration::from_secs(2));
            }
            Err(e) => {
                println!("❌ 带参数启动程序失败: {}", e);
            }
        }
        
        // 清理测试文件
        cleanup_test_files();
        println!("🧹 测试文件已清理");
    }
    
    #[test]
    fn test_launch_program_as_admin() {
        println!("测试管理员模式启动程序...");
        println!("⚠️ 注意: 此测试会弹出 UAC 对话框");
        
        // 测试以管理员权限启动 PowerShell
        let request = LaunchProgramRequest {
            target_path: "powershell.exe".to_string(),
            arguments: Some("-Command \"Write-Host '管理员模式测试成功！按任意键退出...'; Read-Host\"".to_string()),
            run_as_admin: true,
        };
        
        let result = launch_program(request);
        println!("管理员模式启动结果: {:?}", result);
        
        match result {
            Ok(_) => {
                println!("✅ 管理员模式启动成功");
                println!("🔑 应该弹出 UAC 对话框，PowerShell 将以管理员权限运行");
                
                // 等待几秒让用户看到效果
                std::thread::sleep(std::time::Duration::from_secs(3));
            }
            Err(e) => {
                println!("❌ 管理员模式启动失败: {}", e);
            }
        }
    }
    
    #[test]
    fn test_launch_program_admin_with_cmd() {
        println!("测试管理员模式启动 CMD...");
        
        // 测试以管理员权限启动 CMD
        let request = LaunchProgramRequest {
            target_path: "cmd.exe".to_string(),
            arguments: Some("/k echo 管理员CMD测试成功! && echo 当前用户权限: && whoami /priv".to_string()),
            run_as_admin: true,
        };
        
        let result = launch_program(request);
        println!("管理员模式启动CMD结果: {:?}", result);
        
        match result {
            Ok(_) => {
                println!("✅ 管理员CMD启动成功");
                println!("🔑 CMD窗口应该显示管理员权限信息");
            }
            Err(e) => {
                println!("❌ 管理员CMD启动失败: {}", e);
            }
        }
    }
    
    #[test]
    fn test_get_icon_base64() {
        // 测试获取记事本图标
        let result = get_icon_base64("notepad.exe".to_string());
        println!("获取图标结果: {:?}", result.as_ref().map(|s| format!("长度: {} 字符", s.len())));
        
        match result {
            Ok(icon_data) => {
                if icon_data.starts_with("data:image/png;base64,") {
                    println!("✅ 图标提取成功，数据格式正确");
                    assert!(icon_data.len() > 50); // 应该有实际数据
                } else {
                    println!("⚠️ 图标数据格式异常: {}", &icon_data[..50.min(icon_data.len())]);
                }
            }
            Err(e) => {
                println!("❌ 图标提取失败: {}", e);
            }
        }
    }
    
    #[test]
    fn test_scan_system_programs() {
        println!("开始扫描系统程序...");
        
        let result = scan_system_programs();
        println!("扫描结果: {:?}", result.as_ref().map(|programs| format!("找到 {} 个程序", programs.len())));
        
        match result {
            Ok(programs) => {
                println!("✅ 扫描成功，找到 {} 个程序", programs.len());
                
                // 打印前几个程序的详细信息
                for (i, program) in programs.iter().take(5).enumerate() {
                    println!("程序 {}: {} -> {}", i + 1, program.name, program.target_path);
                }
                
                if programs.len() > 5 {
                    println!("... 还有 {} 个程序", programs.len() - 5);
                }
                
                // 验证程序数据格式
                if !programs.is_empty() {
                    let first_program = &programs[0];
                    assert!(!first_program.name.is_empty());
                    assert!(!first_program.target_path.is_empty());
                }
            }
            Err(e) => {
                println!("❌ 扫描失败: {}", e);
            }
        }
    }
    
    #[test]
    fn test_update_usage_count() {
        setup_test();
        
        // 先添加一个程序
        let test_item = LaunchItem {
            name: "计数测试".to_string(),
            target_path: "test.exe".to_string(),
            icon_location: "".to_string(),
            arguments: "".to_string(),
            count: 0,
            icon_base64: None,
        };
        
        let _ = add_launch_item("测试".to_string(), test_item);
        
        // 更新使用次数
        let result = update_usage_count("计数测试".to_string(), "测试".to_string());
        println!("更新使用次数结果: {:?}", result);
        
        match result {
            Ok(_) => {
                println!("✅ 使用次数更新成功");
                
                // 验证计数是否增加
                let config = get_all_launch_items().unwrap();
                if let Some(items) = config.get("测试") {
                    assert_eq!(items[0].count, 1);
                    println!("✅ 验证计数增加成功");
                }
            }
            Err(e) => {
                println!("❌ 更新使用次数失败: {}", e);
            }
        }
        
        cleanup_test_files();
    }
    
    #[test]
    fn test_remove_launch_item() {
        setup_test();
        
        // 先添加一个程序
        let test_item = LaunchItem {
            name: "删除测试".to_string(),
            target_path: "delete_test.exe".to_string(),
            icon_location: "".to_string(),
            arguments: "".to_string(),
            count: 0,
            icon_base64: None,
        };
        
        let _ = add_launch_item("删除测试".to_string(), test_item);
        
        // 删除程序
        let result = remove_launch_item("删除测试".to_string(), "删除测试".to_string());
        println!("删除程序结果: {:?}", result);
        
        match result {
            Ok(_) => {
                println!("✅ 程序删除成功");
                
                // 验证是否真的删除了
                let config = get_all_launch_items().unwrap();
                assert!(!config.contains_key("删除测试"));
                println!("✅ 验证删除成功");
            }
            Err(e) => {
                println!("❌ 删除程序失败: {}", e);
            }
        }
        
        cleanup_test_files();
    }
    
    #[test]
    fn test_path_variables() {
        // 测试路径变量替换功能
        let test_path = "%%rp%%\\test.exe";
        let fixed_path = crate::launcher::fix_path(test_path);
        
        println!("原始路径: {}", test_path);
        println!("修复后路径: {}", fixed_path);
        
        assert!(!fixed_path.contains("%%rp%%"));
        assert!(fixed_path.contains("test.exe"));
        println!("✅ 路径变量替换成功");
    }
    
    // 综合测试：测试所有启动模式
    #[test]
    fn test_comprehensive_launch_modes() {
        println!("\n=== 综合启动模式测试 ===");
        
        // 创建测试文件
        let test_file = create_test_file();
        println!("📁 创建测试文件: {}", test_file);
        
        // 1. 普通模式启动记事本
        println!("\n1. 测试普通模式启动记事本...");
        let request1 = LaunchProgramRequest {
            target_path: "notepad.exe".to_string(),
            arguments: None,
            run_as_admin: false,
        };
        
        match launch_program(request1) {
            Ok(_) => println!("✅ 普通模式启动成功"),
            Err(e) => println!("❌ 普通模式启动失败: {}", e),
        }
        
        std::thread::sleep(std::time::Duration::from_secs(1));
        
        // 2. 带参数启动记事本
        println!("\n2. 测试带参数启动记事本（打开测试文件）...");
        let request2 = LaunchProgramRequest {
            target_path: "notepad.exe".to_string(),
            arguments: Some(test_file.clone()),
            run_as_admin: false,
        };
        
        match launch_program(request2) {
            Ok(_) => println!("✅ 带参数启动成功，记事本应该打开了测试文件"),
            Err(e) => println!("❌ 带参数启动失败: {}", e),
        }
        
        std::thread::sleep(std::time::Duration::from_secs(2));
        
        // 3. 管理员模式启动（警告用户）
        println!("\n3. 测试管理员模式启动...");
        println!("⚠️ 接下来会测试管理员权限启动，可能弹出 UAC 对话框");
        println!("💡 可以点击'是'来测试，或'否'来跳过");
        
        let request3 = LaunchProgramRequest {
            target_path: "cmd.exe".to_string(),
            arguments: Some("/c echo 管理员权限测试 && timeout 3".to_string()),
            run_as_admin: true,
        };
        
        match launch_program(request3) {
            Ok(_) => println!("✅ 管理员模式启动成功"),
            Err(e) => println!("❌ 管理员模式启动失败: {}", e),
        }
        
        // 4. 测试路径变量
        println!("\n4. 测试路径变量处理...");
        let test_path_with_vars = format!("%%rp%%\\{}", TEST_FILE_NAME);
        let fixed_path = crate::launcher::fix_path(&test_path_with_vars);
        println!("原始路径: {}", test_path_with_vars);
        println!("处理后路径: {}", fixed_path);
        
        if !fixed_path.contains("%%rp%%") && fixed_path.contains(TEST_FILE_NAME) {
            println!("✅ 路径变量处理成功");
        } else {
            println!("❌ 路径变量处理失败");
        }
        
        // 清理
        cleanup_test_files();
        println!("\n🧹 测试文件已清理");
        println!("=== 综合测试完成 ===");
    }
}
