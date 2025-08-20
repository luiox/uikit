use wrapper_assistant_lib::{launch_program_impl, LaunchProgramRequest};

#[test]
fn test_launch_program() {
    launch_program_impl(LaunchProgramRequest {
        target_path: r#"D:\MyToolKit\Notepad++\notepad++.exe"#.to_string(),
        arguments: Some("test.txt".to_string()),
        run_as_admin: true,
    }).expect("Failed to launch program");
}
