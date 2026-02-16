export interface LaunchItem {
  id: string;
  itemType: "app" | "separator" | string;
  name: string;
  targetPath: string;
  iconLocation: string;
  arguments: string;
  launchCount: number;
  enabled: boolean;
}

export interface Group {
  id: string;
  name: string;
  order: number;
  items: LaunchItem[];
}

export interface Settings {
  hotkey: string;
  executeHide: boolean;
  currentGroup: string | null;
}

export interface LauncherState {
  groups: Group[];
  settings: Settings;
}

export interface EditorContext {
  groupId: string;
  item?: LaunchItem;
}
