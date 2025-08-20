
#[test]
fn test_launch_program() {
    // launch_program_impl(LaunchProgramRequest {
    //     target_path: r#"D:\MyToolKit\Notepad++\notepad++.exe"#.to_string(),
    //     arguments: Some("test.txt".to_string()),
    //     run_as_admin: true,
    // }).expect("Failed to launch program");
}

#[test]
fn test_get_launcher_absolute_dir() {
    use wrapper_assistant_lib::get_launcher_absolute_dir;
    let dir = get_launcher_absolute_dir();
    print!( "Launcher absolute dir: {}", dir);
    assert!(!dir.is_empty());
}

#[test]
fn test_get_launcher_root_dir() {
    use wrapper_assistant_lib::get_launcher_root_dir;
    let dir = get_launcher_root_dir();
    print!( "Launcher root dir: {}", dir);
    assert!(!dir.is_empty());
}