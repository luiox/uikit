use serde::Deserialize;

#[derive(Deserialize)]
pub struct LaunchProgramRequest {
    pub target_path: String,
    pub arguments: Option<String>,
    pub run_as_admin: bool,
}

pub fn launch_program_impl(req: LaunchProgramRequest) -> Result<(), String> {
    if req.run_as_admin {
        // Windows下用ShellExecuteW实现管理员提权启动
        #[cfg(target_os = "windows")]
        {
            use windows::Win32::UI::Shell::ShellExecuteW;
            use windows::Win32::Foundation::{HWND, HINSTANCE};
            use windows::core::PCWSTR;
            use std::ffi::OsStr;
            use std::os::windows::ffi::OsStrExt;
            use windows::Win32::UI::WindowsAndMessaging::SW_SHOWNORMAL;

            let exe: Vec<u16> = OsStr::new(&req.target_path).encode_wide().chain(Some(0)).collect();
            let args: Vec<u16> = req.arguments.as_deref().map(|s| OsStr::new(s).encode_wide().chain(Some(0)).collect()).unwrap_or_else(|| vec![0]);
            let verb: Vec<u16> = OsStr::new("runas").encode_wide().chain(Some(0)).collect();

            let result = unsafe {
                ShellExecuteW(
                    Some(HWND(std::ptr::null_mut())),
                    PCWSTR(verb.as_ptr()),
                    PCWSTR(exe.as_ptr()),
                    if args.len() > 1 { PCWSTR(args.as_ptr()) } else { PCWSTR(std::ptr::null()) },
                    PCWSTR(std::ptr::null()),
                    SW_SHOWNORMAL, 
                )
            };
            if (result.0 as usize) <= 32 {
                return Err(format!("ShellExecuteW failed: code {}", result.0 as usize));
            }
            Ok(())
        }
        #[cfg(not(target_os = "windows"))]
        {
            Err("管理员模式仅支持 Windows".to_string())
        }
    } else {
        // 普通启动
        let mut cmd = std::process::Command::new(&req.target_path);
        if let Some(args) = &req.arguments {
            // 简单分割参数，复杂场景可用 shell-words crate
            for arg in args.split_whitespace() {
                cmd.arg(arg);
            }
        }
        cmd.spawn()
            .map(|_| ())
            .map_err(|e| format!("启动失败: {}", e))
    }
}
