import { invoke } from "@tauri-apps/api/core";
import { listen } from "@tauri-apps/api/event";
import { getCurrentWebviewWindow } from "@tauri-apps/api/webviewWindow";
import type { EditorContext, LauncherState, LaunchItem, Settings } from "./types";

export async function loadLauncherState(): Promise<LauncherState> {
  return invoke("load_launcher_state");
}

export async function openEditor(groupId: string, itemId: string | null) {
  return invoke("open_editor", { groupId, itemId });
}

export async function deleteItem(groupId: string, itemId: string) {
  return invoke("delete_item", { groupId, itemId });
}

export async function launchItem(groupId: string, itemId: string): Promise<{ ok: boolean; message: string }> {
  return invoke("launch_item", { groupId, itemId });
}

export async function updateSettings(settings: Settings) {
  return invoke("update_settings", { settings });
}

export async function upsertItem(groupId: string, item: Partial<LaunchItem> & { name: string; targetPath: string; iconLocation: string; arguments: string; itemType: string; enabled?: boolean; id?: string | null }) {
  return invoke("upsert_item", { groupId, item });
}

export async function getEditorContext(): Promise<EditorContext | null> {
  return invoke("get_editor_context");
}

export async function closeEditor() {
  return invoke("close_editor");
}

export async function addGroup(name: string): Promise<string> {
  return invoke("add_group", { name });
}

export async function hideCurrentWindow() {
  const window = getCurrentWebviewWindow();
  await window.hide();
}

export async function onDataChanged(handler: () => void) {
  return listen("data-changed", handler);
}

export async function onSettingsChanged(handler: () => void) {
  return listen("settings-changed", handler);
}

export async function onEditorContextChanged(handler: () => void) {
  return listen("editor-context-changed", handler);
}
