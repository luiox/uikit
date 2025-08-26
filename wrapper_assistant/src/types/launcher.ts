export interface LaunchItem {
  name: string;
  target_path: string;
  icon_location: string;
  arguments: string;
  count: number;
  icon_base64?: string;
}

export interface LaunchProgramRequest {
  target_path: string;
  arguments?: string;
  run_as_admin: boolean;
}

export type LaunchConfig = Record<string, LaunchItem[]>;

export interface LaunchItemWithCategory extends LaunchItem {
  category: string;
}
