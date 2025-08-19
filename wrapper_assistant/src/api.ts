import { invoke } from "@tauri-apps/api/core";

export interface LaunchItem {
  category: string;
  name: string;
  target_path: string;
  icon_location: string;
  arguments: string;
  count: number;
  icon_base64?: string;
}

export interface LaunchConfig {
  categories: string[];
  items: LaunchItem[];
}

export async function fetchLaunchConfig(): Promise<LaunchConfig> {
  return await invoke<LaunchConfig>('get_all_launch_items');
}

export async function fetchIconBase64(path: string): Promise<string> {
  return await invoke<string>('get_icon_base64', { path });
}

export async function saveLaunchConfig(config: LaunchConfig): Promise<void> {
  return await invoke('save_config', { newConfig: config });
}
