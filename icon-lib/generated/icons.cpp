#include "icons.h"

#include <cstring>

namespace iconlib {

#if ICONLIB_ENABLE_EMBED
static const char kSvg_Close[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"currentColor\"><path d=\"M11.9997 10.5865L16.9495 5.63672L18.3637 7.05093L13.4139 12.0007L18.3637 16.9504L16.9495 18.3646L11.9997 13.4149L7.04996 18.3646L5.63574 16.9504L10.5855 12.0007L5.63574 7.05093L7.04996 5.63672L11.9997 10.5865Z\"></path></svg>";
static const char kSvg_Minimize[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-minimize\"><path d=\"M8 3v3a2 2 0 0 1-2 2H3m18 0h-3a2 2 0 0 1-2-2V3m0 18v-3a2 2 0 0 1 2-2h3M3 16h3a2 2 0 0 1 2 2v3\"></path></svg>";
static const char kSvg_Maximize[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-maximize\"><path d=\"M8 3H5a2 2 0 0 0-2 2v3m18 0V5a2 2 0 0 0-2-2h-3m0 18h3a2 2 0 0 0 2-2v-3M3 16v3a2 2 0 0 0 2 2h3\"></path></svg>";
static const char kSvg_Info[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-info\"><circle cx=\"12\" cy=\"12\" r=\"10\"></circle><line x1=\"12\" y1=\"16\" x2=\"12\" y2=\"12\"></line><line x1=\"12\" y1=\"8\" x2=\"12.01\" y2=\"8\"></line></svg>";
static const char kSvg_Warning[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"currentColor\"><path d=\"M12 22C6.47715 22 2 17.5228 2 12C2 6.47715 6.47715 2 12 2C17.5228 2 22 6.47715 22 12C22 17.5228 17.5228 22 12 22ZM12 20C16.4183 20 20 16.4183 20 12C20 7.58172 16.4183 4 12 4C7.58172 4 4 7.58172 4 12C4 16.4183 7.58172 20 12 20ZM11 15H13V17H11V15ZM11 7H13V13H11V7Z\"></path></svg>";
static const char kSvg_Error[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"currentColor\"><path d=\"M12 22C6.47715 22 2 17.5228 2 12C2 6.47715 6.47715 2 12 2C17.5228 2 22 6.47715 22 12C22 17.5228 17.5228 22 12 22ZM11 15V17H13V15H11ZM11 7V13H13V7H11Z\"></path></svg>";
static const char kSvg_Settings[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-settings\"><circle cx=\"12\" cy=\"12\" r=\"3\"></circle><path d=\"M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z\"></path></svg>";
static const char kSvg_Menu[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-menu\"><line x1=\"3\" y1=\"12\" x2=\"21\" y2=\"12\"></line><line x1=\"3\" y1=\"6\" x2=\"21\" y2=\"6\"></line><line x1=\"3\" y1=\"18\" x2=\"21\" y2=\"18\"></line></svg>";
static const char kSvg_File[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-file\"><path d=\"M13 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V9z\"></path><polyline points=\"13 2 13 9 20 9\"></polyline></svg>";
static const char kSvg_Folder[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-folder\"><path d=\"M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z\"></path></svg>";
static const char kSvg_FolderOpen[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"currentColor\"><path d=\"M3 21C2.44772 21 2 20.5523 2 20V4C2 3.44772 2.44772 3 3 3H10.4142L12.4142 5H20C20.5523 5 21 5.44772 21 6V9H19V7H11.5858L9.58579 5H4V16.998L5.5 11H22.5L20.1894 20.2425C20.0781 20.6877 19.6781 21 19.2192 21H3ZM19.9384 13H7.06155L5.56155 19H18.4384L19.9384 13Z\"></path></svg>";
static const char kSvg_Image[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-image\"><rect x=\"3\" y=\"3\" width=\"18\" height=\"18\" rx=\"2\" ry=\"2\"></rect><circle cx=\"8.5\" cy=\"8.5\" r=\"1.5\"></circle><polyline points=\"21 15 16 10 5 21\"></polyline></svg>";
static const char kSvg_Video[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-video\"><polygon points=\"23 7 16 12 23 17 23 7\"></polygon><rect x=\"1\" y=\"5\" width=\"15\" height=\"14\" rx=\"2\" ry=\"2\"></rect></svg>";
static const char kSvg_Archive[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-archive\"><polyline points=\"21 8 21 21 3 21 3 8\"></polyline><rect x=\"1\" y=\"3\" width=\"22\" height=\"5\"></rect><line x1=\"10\" y1=\"12\" x2=\"14\" y2=\"12\"></line></svg>";
static const char kSvg_Code[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-code\"><polyline points=\"16 18 22 12 16 6\"></polyline><polyline points=\"8 6 2 12 8 18\"></polyline></svg>";
static const char kSvg_New[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-plus\"><line x1=\"12\" y1=\"5\" x2=\"12\" y2=\"19\"></line><line x1=\"5\" y1=\"12\" x2=\"19\" y2=\"12\"></line></svg>";
static const char kSvg_Open[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-folder\"><path d=\"M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z\"></path></svg>";
static const char kSvg_Save[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-save\"><path d=\"M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11a2 2 0 0 1-2 2z\"></path><polyline points=\"17 21 17 13 7 13 7 21\"></polyline><polyline points=\"7 3 7 8 15 8\"></polyline></svg>";
static const char kSvg_Delete[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-delete\"><path d=\"M21 4H8l-7 8 7 8h13a2 2 0 0 0 2-2V6a2 2 0 0 0-2-2z\"></path><line x1=\"18\" y1=\"9\" x2=\"12\" y2=\"15\"></line><line x1=\"12\" y1=\"9\" x2=\"18\" y2=\"15\"></line></svg>";
static const char kSvg_Edit[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-edit\"><path d=\"M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7\"></path><path d=\"M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z\"></path></svg>";
static const char kSvg_Copy[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-copy\"><rect x=\"9\" y=\"9\" width=\"13\" height=\"13\" rx=\"2\" ry=\"2\"></rect><path d=\"M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1\"></path></svg>";
static const char kSvg_Undo[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"currentColor\"><path d=\"M5.82843 6.99955L8.36396 9.53509L6.94975 10.9493L2 5.99955L6.94975 1.0498L8.36396 2.46402L5.82843 4.99955H13C17.4183 4.99955 21 8.58127 21 12.9996C21 17.4178 17.4183 20.9996 13 20.9996H4V18.9996H13C16.3137 18.9996 19 16.3133 19 12.9996C19 9.68584 16.3137 6.99955 13 6.99955H5.82843Z\"></path></svg>";
static const char kSvg_Redo[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"currentColor\"><path d=\"M18.1716 6.99955H11C7.68629 6.99955 5 9.68584 5 12.9996C5 16.3133 7.68629 18.9996 11 18.9996H20V20.9996H11C6.58172 20.9996 3 17.4178 3 12.9996C3 8.58127 6.58172 4.99955 11 4.99955H18.1716L15.636 2.46402L17.0503 1.0498L22 5.99955L17.0503 10.9493L15.636 9.53509L18.1716 6.99955Z\"></path></svg>";
static const char kSvg_Find[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-search\"><circle cx=\"11\" cy=\"11\" r=\"8\"></circle><line x1=\"21\" y1=\"21\" x2=\"16.65\" y2=\"16.65\"></line></svg>";
static const char kSvg_Replace[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-refresh-cw\"><polyline points=\"23 4 23 10 17 10\"></polyline><polyline points=\"1 20 1 14 7 14\"></polyline><path d=\"M3.51 9a9 9 0 0 1 14.85-3.36L23 10M1 14l4.64 4.36A9 9 0 0 0 20.49 15\"></path></svg>";
static const char kSvg_Back[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-arrow-left\"><line x1=\"19\" y1=\"12\" x2=\"5\" y2=\"12\"></line><polyline points=\"12 19 5 12 12 5\"></polyline></svg>";
static const char kSvg_Forward[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-arrow-right\"><line x1=\"5\" y1=\"12\" x2=\"19\" y2=\"12\"></line><polyline points=\"12 5 19 12 12 19\"></polyline></svg>";
static const char kSvg_Up[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-arrow-up\"><line x1=\"12\" y1=\"19\" x2=\"12\" y2=\"5\"></line><polyline points=\"5 12 12 5 19 12\"></polyline></svg>";
static const char kSvg_Refresh[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-refresh-cw\"><polyline points=\"23 4 23 10 17 10\"></polyline><polyline points=\"1 20 1 14 7 14\"></polyline><path d=\"M3.51 9a9 9 0 0 1 14.85-3.36L23 10M1 14l4.64 4.36A9 9 0 0 0 20.49 15\"></path></svg>";
static const char kSvg_Home[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-home\"><path d=\"M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z\"></path><polyline points=\"9 22 9 12 15 12 15 22\"></polyline></svg>";
static const char kSvg_ZoomIn[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-zoom-in\"><circle cx=\"11\" cy=\"11\" r=\"8\"></circle><line x1=\"21\" y1=\"21\" x2=\"16.65\" y2=\"16.65\"></line><line x1=\"11\" y1=\"8\" x2=\"11\" y2=\"14\"></line><line x1=\"8\" y1=\"11\" x2=\"14\" y2=\"11\"></line></svg>";
static const char kSvg_ZoomOut[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-zoom-out\"><circle cx=\"11\" cy=\"11\" r=\"8\"></circle><line x1=\"21\" y1=\"21\" x2=\"16.65\" y2=\"16.65\"></line><line x1=\"8\" y1=\"11\" x2=\"14\" y2=\"11\"></line></svg>";
static const char kSvg_Fullscreen[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-maximize\"><path d=\"M8 3H5a2 2 0 0 0-2 2v3m18 0V5a2 2 0 0 0-2-2h-3m0 18h3a2 2 0 0 0 2-2v-3M3 16v3a2 2 0 0 0 2 2h3\"></path></svg>";
static const char kSvg_FullscreenExit[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-minimize\"><path d=\"M8 3v3a2 2 0 0 1-2 2H3m18 0h-3a2 2 0 0 1-2-2V3m0 18v-3a2 2 0 0 1 2-2h3M3 16h3a2 2 0 0 1 2 2v3\"></path></svg>";
static const char kSvg_Grid[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-grid\"><rect x=\"3\" y=\"3\" width=\"7\" height=\"7\"></rect><rect x=\"14\" y=\"3\" width=\"7\" height=\"7\"></rect><rect x=\"14\" y=\"14\" width=\"7\" height=\"7\"></rect><rect x=\"3\" y=\"14\" width=\"7\" height=\"7\"></rect></svg>";
static const char kSvg_Search[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-search\"><circle cx=\"11\" cy=\"11\" r=\"8\"></circle><line x1=\"21\" y1=\"21\" x2=\"16.65\" y2=\"16.65\"></line></svg>";
static const char kSvg_Filter[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-filter\"><polygon points=\"22 3 2 3 10 12.46 10 19 14 21 14 12.46 22 3\"></polygon></svg>";
static const char kSvg_SortAsc[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-arrow-up\"><line x1=\"12\" y1=\"19\" x2=\"12\" y2=\"5\"></line><polyline points=\"5 12 12 5 19 12\"></polyline></svg>";
static const char kSvg_SortDesc[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-arrow-down\"><line x1=\"12\" y1=\"5\" x2=\"12\" y2=\"19\"></line><polyline points=\"19 12 12 19 5 12\"></polyline></svg>";
static const char kSvg_Check[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-check\"><polyline points=\"20 6 9 17 4 12\"></polyline></svg>";
static const char kSvg_Circle[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-circle\"><circle cx=\"12\" cy=\"12\" r=\"10\"></circle></svg>";
static const char kSvg_Square[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-square\"><rect x=\"3\" y=\"3\" width=\"18\" height=\"18\" rx=\"2\" ry=\"2\"></rect></svg>";
static const char kSvg_Star[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-star\"><polygon points=\"12 2 15.09 8.26 22 9.27 17 14.14 18.18 21.02 12 17.77 5.82 21.02 7 14.14 2 9.27 8.91 8.26 12 2\"></polygon></svg>";
static const char kSvg_Heart[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-heart\"><path d=\"M20.84 4.61a5.5 5.5 0 0 0-7.78 0L12 5.67l-1.06-1.06a5.5 5.5 0 0 0-7.78 7.78l1.06 1.06L12 21.23l7.78-7.78 1.06-1.06a5.5 5.5 0 0 0 0-7.78z\"></path></svg>";
static const char kSvg_Flag[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-flag\"><path d=\"M4 15s1-1 4-1 5 2 8 2 4-1 4-1V3s-1 1-4 1-5-2-8-2-4 1-4 1z\"></path><line x1=\"4\" y1=\"22\" x2=\"4\" y2=\"15\"></line></svg>";
static const char kSvg_Tag[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-tag\"><path d=\"M20.59 13.41l-7.17 7.17a2 2 0 0 1-2.83 0L2 12V2h10l8.59 8.59a2 2 0 0 1 0 2.82z\"></path><line x1=\"7\" y1=\"7\" x2=\"7.01\" y2=\"7\"></line></svg>";
static const char kSvg_Clock[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-clock\"><circle cx=\"12\" cy=\"12\" r=\"10\"></circle><polyline points=\"12 6 12 12 16 14\"></polyline></svg>";
static const char kSvg_Calendar[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-calendar\"><rect x=\"3\" y=\"4\" width=\"18\" height=\"18\" rx=\"2\" ry=\"2\"></rect><line x1=\"16\" y1=\"2\" x2=\"16\" y2=\"6\"></line><line x1=\"8\" y1=\"2\" x2=\"8\" y2=\"6\"></line><line x1=\"3\" y1=\"10\" x2=\"21\" y2=\"10\"></line></svg>";
static const char kSvg_Link[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-link\"><path d=\"M10 13a5 5 0 0 0 7.54.54l3-3a5 5 0 0 0-7.07-7.07l-1.72 1.71\"></path><path d=\"M14 11a5 5 0 0 0-7.54-.54l-3 3a5 5 0 0 0 7.07 7.07l1.71-1.71\"></path></svg>";
static const char kSvg_Web[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-globe\"><circle cx=\"12\" cy=\"12\" r=\"10\"></circle><line x1=\"2\" y1=\"12\" x2=\"22\" y2=\"12\"></line><path d=\"M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z\"></path></svg>";
static const char kSvg_Download[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-download\"><path d=\"M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4\"></path><polyline points=\"7 10 12 15 17 10\"></polyline><line x1=\"12\" y1=\"15\" x2=\"12\" y2=\"3\"></line></svg>";
static const char kSvg_Upload[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-upload\"><path d=\"M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4\"></path><polyline points=\"17 8 12 3 7 8\"></polyline><line x1=\"12\" y1=\"3\" x2=\"12\" y2=\"15\"></line></svg>";
static const char kSvg_Cloud[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-cloud\"><path d=\"M18 10h-1.26A8 8 0 1 0 9 20h9a5 5 0 0 0 0-10z\"></path></svg>";
static const char kSvg_Share[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-share\"><path d=\"M4 12v8a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2v-8\"></path><polyline points=\"16 6 12 2 8 6\"></polyline><line x1=\"12\" y1=\"2\" x2=\"12\" y2=\"15\"></line></svg>";
static const char kSvg_External[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-external-link\"><path d=\"M18 13v6a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2h6\"></path><polyline points=\"15 3 21 3 21 9\"></polyline><line x1=\"10\" y1=\"14\" x2=\"21\" y2=\"3\"></line></svg>";
static const char kSvg_Play[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-play\"><polygon points=\"5 3 19 12 5 21 5 3\"></polygon></svg>";
static const char kSvg_Pause[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-pause\"><rect x=\"6\" y=\"4\" width=\"4\" height=\"16\"></rect><rect x=\"14\" y=\"4\" width=\"4\" height=\"16\"></rect></svg>";
static const char kSvg_Prev[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-skip-back\"><polygon points=\"19 20 9 12 19 4 19 20\"></polygon><line x1=\"5\" y1=\"19\" x2=\"5\" y2=\"5\"></line></svg>";
static const char kSvg_Next[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-skip-forward\"><polygon points=\"5 4 15 12 5 20 5 4\"></polygon><line x1=\"19\" y1=\"5\" x2=\"19\" y2=\"19\"></line></svg>";
static const char kSvg_Volume[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-volume\"><polygon points=\"11 5 6 9 2 9 2 15 6 15 11 19 11 5\"></polygon></svg>";
static const char kSvg_VolumeMute[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-volume-x\"><polygon points=\"11 5 6 9 2 9 2 15 6 15 11 19 11 5\"></polygon><line x1=\"23\" y1=\"9\" x2=\"17\" y2=\"15\"></line><line x1=\"17\" y1=\"9\" x2=\"23\" y2=\"15\"></line></svg>";
static const char kSvg_Mic[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-mic\"><path d=\"M12 1a3 3 0 0 0-3 3v8a3 3 0 0 0 6 0V4a3 3 0 0 0-3-3z\"></path><path d=\"M19 10v2a7 7 0 0 1-14 0v-2\"></path><line x1=\"12\" y1=\"19\" x2=\"12\" y2=\"23\"></line><line x1=\"8\" y1=\"23\" x2=\"16\" y2=\"23\"></line></svg>";
static const char kSvg_Computer[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-monitor\"><rect x=\"2\" y=\"3\" width=\"20\" height=\"14\" rx=\"2\" ry=\"2\"></rect><line x1=\"8\" y1=\"21\" x2=\"16\" y2=\"21\"></line><line x1=\"12\" y1=\"17\" x2=\"12\" y2=\"21\"></line></svg>";
static const char kSvg_Cmd[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-terminal\"><polyline points=\"4 17 10 11 4 5\"></polyline><line x1=\"12\" y1=\"19\" x2=\"20\" y2=\"19\"></line></svg>";
static const char kSvg_Shutdown[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-power\"><path d=\"M18.36 6.64a9 9 0 1 1-12.73 0\"></path><line x1=\"12\" y1=\"2\" x2=\"12\" y2=\"12\"></line></svg>";
static const char kSvg_Restart[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-rotate-cw\"><polyline points=\"23 4 23 10 17 10\"></polyline><path d=\"M20.49 15a9 9 0 1 1-2.12-9.36L23 10\"></path></svg>";
static const char kSvg_Lock[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-lock\"><rect x=\"3\" y=\"11\" width=\"18\" height=\"11\" rx=\"2\" ry=\"2\"></rect><path d=\"M7 11V7a5 5 0 0 1 10 0v4\"></path></svg>";
static const char kSvg_Unlock[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-unlock\"><rect x=\"3\" y=\"11\" width=\"18\" height=\"11\" rx=\"2\" ry=\"2\"></rect><path d=\"M7 11V7a5 5 0 0 1 9.9-1\"></path></svg>";
static const char kSvg_Key[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-key\"><path d=\"M21 2l-2 2m-7.61 7.61a5.5 5.5 0 1 1-7.778 7.778 5.5 5.5 0 0 1 7.777-7.777zm0 0L15.5 7.5m0 0l3 3L22 7l-3-3m-3.5 3.5L19 4\"></path></svg>";
static const char kSvg_Printer[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-printer\"><polyline points=\"6 9 6 2 18 2 18 9\"></polyline><path d=\"M6 18H4a2 2 0 0 1-2-2v-5a2 2 0 0 1 2-2h16a2 2 0 0 1 2 2v5a2 2 0 0 1-2 2h-2\"></path><rect x=\"6\" y=\"14\" width=\"12\" height=\"8\"></rect></svg>";
static const char kSvg_Battery[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-battery\"><rect x=\"1\" y=\"6\" width=\"18\" height=\"12\" rx=\"2\" ry=\"2\"></rect><line x1=\"23\" y1=\"13\" x2=\"23\" y2=\"11\"></line></svg>";
static const char kSvg_Wifi[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-wifi\"><path d=\"M5 12.55a11 11 0 0 1 14.08 0\"></path><path d=\"M1.42 9a16 16 0 0 1 21.16 0\"></path><path d=\"M8.53 16.11a6 6 0 0 1 6.95 0\"></path><line x1=\"12\" y1=\"20\" x2=\"12.01\" y2=\"20\"></line></svg>";
static const char kSvg_App[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-box\"><path d=\"M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16z\"></path><polyline points=\"3.27 6.96 12 12.01 20.73 6.96\"></polyline><line x1=\"12\" y1=\"22.08\" x2=\"12\" y2=\"12\"></line></svg>";
static const char kSvg_Pin[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"currentColor\"><path d=\"M13.8273 1.69L22.3126 10.1753L20.8984 11.5895L20.1913 10.8824L15.9486 15.125L15.2415 18.6606L13.8273 20.0748L9.58466 15.8321L4.63492 20.7819L3.2207 19.3677L8.17045 14.4179L3.92781 10.1753L5.34202 8.76107L8.87756 8.05396L13.1202 3.81132L12.4131 3.10422L13.8273 1.69ZM14.5344 5.22554L9.86358 9.89637L7.0417 10.4607L13.5418 16.9609L14.1062 14.139L18.7771 9.46818L14.5344 5.22554Z\"></path></svg>";
static const char kSvg_Unpin[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"currentColor\"><path d=\"M18 3V5H17V11L19 14V16H13V23H11V16H5V14L7 11V5H6V3H18ZM9 5V11.6056L7.4037 14H16.5963L15 11.6056V5H9Z\"></path></svg>";
static const char kSvg_Plus[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-plus\"><line x1=\"12\" y1=\"5\" x2=\"12\" y2=\"19\"></line><line x1=\"5\" y1=\"12\" x2=\"19\" y2=\"12\"></line></svg>";
static const char kSvg_Minus[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-minus\"><line x1=\"5\" y1=\"12\" x2=\"19\" y2=\"12\"></line></svg>";
static const char kSvg_History[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-clock\"><circle cx=\"12\" cy=\"12\" r=\"10\"></circle><polyline points=\"12 6 12 12 16 14\"></polyline></svg>";
static const char kSvg_Clear[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-x-circle\"><circle cx=\"12\" cy=\"12\" r=\"10\"></circle><line x1=\"15\" y1=\"9\" x2=\"9\" y2=\"15\"></line><line x1=\"9\" y1=\"9\" x2=\"15\" y2=\"15\"></line></svg>";
#endif

static const IconAsset kIcons[] = {
#if ICONLIB_ENABLE_EMBED
    {Icon::Close, "close", "icons/close.svg", kSvg_Close, 313},
#else
    {Icon::Close, "close", "icons/close.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Minimize, "minimize", "icons/minimize.svg", kSvg_Minimize, 331},
#else
    {Icon::Minimize, "minimize", "icons/minimize.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Maximize, "maximize", "icons/maximize.svg", kSvg_Maximize, 331},
#else
    {Icon::Maximize, "maximize", "icons/maximize.svg", nullptr, 0},
#endif
    {Icon::Restore, "restore", "icons/restore.svg", nullptr, 0},
    {Icon::Help, "help", "icons/help.svg", nullptr, 0},
#if ICONLIB_ENABLE_EMBED
    {Icon::Info, "info", "icons/info.svg", kSvg_Info, 347},
#else
    {Icon::Info, "info", "icons/info.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Warning, "warning", "icons/warning.svg", kSvg_Warning, 366},
#else
    {Icon::Warning, "warning", "icons/warning.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Error, "error", "icons/error.svg", kSvg_Error, 253},
#else
    {Icon::Error, "error", "icons/error.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Settings, "settings", "icons/settings.svg", kSvg_Settings, 1011},
#else
    {Icon::Settings, "settings", "icons/settings.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Menu, "menu", "icons/menu.svg", kSvg_Menu, 346},
#else
    {Icon::Menu, "menu", "icons/menu.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::File, "file", "icons/file.svg", kSvg_File, 337},
#else
    {Icon::File, "file", "icons/file.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Folder, "folder", "icons/folder.svg", kSvg_Folder, 311},
#else
    {Icon::Folder, "folder", "icons/folder.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::FolderOpen, "folder_open", "icons/folder_open.svg", kSvg_FolderOpen, 361},
#else
    {Icon::FolderOpen, "folder_open", "icons/folder_open.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Image, "image", "icons/image.svg", kSvg_Image, 369},
#else
    {Icon::Image, "image", "icons/image.svg", nullptr, 0},
#endif
    {Icon::Audio, "audio", "icons/audio.svg", nullptr, 0},
#if ICONLIB_ENABLE_EMBED
    {Icon::Video, "video", "icons/video.svg", kSvg_Video, 329},
#else
    {Icon::Video, "video", "icons/video.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Archive, "archive", "icons/archive.svg", kSvg_Archive, 361},
#else
    {Icon::Archive, "archive", "icons/archive.svg", nullptr, 0},
#endif
    {Icon::Pdf, "pdf", "icons/pdf.svg", nullptr, 0},
    {Icon::Word, "word", "icons/word.svg", nullptr, 0},
    {Icon::Excel, "excel", "icons/excel.svg", nullptr, 0},
    {Icon::Ppt, "ppt", "icons/ppt.svg", nullptr, 0},
#if ICONLIB_ENABLE_EMBED
    {Icon::Code, "code", "icons/code.svg", kSvg_Code, 307},
#else
    {Icon::Code, "code", "icons/code.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::New, "new", "icons/new.svg", kSvg_New, 304},
#else
    {Icon::New, "new", "icons/new.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Open, "open", "icons/open.svg", kSvg_Open, 311},
#else
    {Icon::Open, "open", "icons/open.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Save, "save", "icons/save.svg", kSvg_Save, 392},
#else
    {Icon::Save, "save", "icons/save.svg", nullptr, 0},
#endif
    {Icon::SaveAs, "save_as", "icons/save_as.svg", nullptr, 0},
#if ICONLIB_ENABLE_EMBED
    {Icon::Delete, "delete", "icons/delete.svg", kSvg_Delete, 374},
#else
    {Icon::Delete, "delete", "icons/delete.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Edit, "edit", "icons/edit.svg", kSvg_Edit, 365},
#else
    {Icon::Edit, "edit", "icons/edit.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Copy, "copy", "icons/copy.svg", kSvg_Copy, 351},
#else
    {Icon::Copy, "copy", "icons/copy.svg", nullptr, 0},
#endif
    {Icon::Paste, "paste", "icons/paste.svg", nullptr, 0},
    {Icon::Cut, "cut", "icons/cut.svg", nullptr, 0},
#if ICONLIB_ENABLE_EMBED
    {Icon::Undo, "undo", "icons/undo.svg", kSvg_Undo, 386},
#else
    {Icon::Undo, "undo", "icons/undo.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Redo, "redo", "icons/redo.svg", kSvg_Redo, 380},
#else
    {Icon::Redo, "redo", "icons/redo.svg", nullptr, 0},
#endif
    {Icon::SelectAll, "select_all", "icons/select_all.svg", nullptr, 0},
#if ICONLIB_ENABLE_EMBED
    {Icon::Find, "find", "icons/find.svg", kSvg_Find, 308},
#else
    {Icon::Find, "find", "icons/find.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Replace, "replace", "icons/replace.svg", kSvg_Replace, 400},
#else
    {Icon::Replace, "replace", "icons/replace.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Back, "back", "icons/back.svg", kSvg_Back, 312},
#else
    {Icon::Back, "back", "icons/back.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Forward, "forward", "icons/forward.svg", kSvg_Forward, 314},
#else
    {Icon::Forward, "forward", "icons/forward.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Up, "up", "icons/up.svg", kSvg_Up, 310},
#else
    {Icon::Up, "up", "icons/up.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Refresh, "refresh", "icons/refresh.svg", kSvg_Refresh, 400},
#else
    {Icon::Refresh, "refresh", "icons/refresh.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Home, "home", "icons/home.svg", kSvg_Home, 332},
#else
    {Icon::Home, "home", "icons/home.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::ZoomIn, "zoom_in", "icons/zoom_in.svg", kSvg_ZoomIn, 397},
#else
    {Icon::ZoomIn, "zoom_in", "icons/zoom_in.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::ZoomOut, "zoom_out", "icons/zoom_out.svg", kSvg_ZoomOut, 354},
#else
    {Icon::ZoomOut, "zoom_out", "icons/zoom_out.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Fullscreen, "fullscreen", "icons/fullscreen.svg", kSvg_Fullscreen, 331},
#else
    {Icon::Fullscreen, "fullscreen", "icons/fullscreen.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::FullscreenExit, "fullscreen_exit", "icons/fullscreen_exit.svg", kSvg_FullscreenExit, 331},
#else
    {Icon::FullscreenExit, "fullscreen_exit", "icons/fullscreen_exit.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Grid, "grid", "icons/grid.svg", kSvg_Grid, 404},
#else
    {Icon::Grid, "grid", "icons/grid.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Search, "search", "icons/search.svg", kSvg_Search, 308},
#else
    {Icon::Search, "search", "icons/search.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Filter, "filter", "icons/filter.svg", kSvg_Filter, 290},
#else
    {Icon::Filter, "filter", "icons/filter.svg", nullptr, 0},
#endif
    {Icon::Sort, "sort", "icons/sort.svg", nullptr, 0},
#if ICONLIB_ENABLE_EMBED
    {Icon::SortAsc, "sort_asc", "icons/sort_asc.svg", kSvg_SortAsc, 310},
#else
    {Icon::SortAsc, "sort_asc", "icons/sort_asc.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::SortDesc, "sort_desc", "icons/sort_desc.svg", kSvg_SortDesc, 313},
#else
    {Icon::SortDesc, "sort_desc", "icons/sort_desc.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Check, "check", "icons/check.svg", kSvg_Check, 262},
#else
    {Icon::Check, "check", "icons/check.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Circle, "circle", "icons/circle.svg", kSvg_Circle, 258},
#else
    {Icon::Circle, "circle", "icons/circle.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Square, "square", "icons/square.svg", kSvg_Square, 280},
#else
    {Icon::Square, "square", "icons/square.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Star, "star", "icons/star.svg", kSvg_Star, 339},
#else
    {Icon::Star, "star", "icons/star.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Heart, "heart", "icons/heart.svg", kSvg_Heart, 371},
#else
    {Icon::Heart, "heart", "icons/heart.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Flag, "flag", "icons/flag.svg", kSvg_Flag, 334},
#else
    {Icon::Flag, "flag", "icons/flag.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Tag, "tag", "icons/tag.svg", kSvg_Tag, 355},
#else
    {Icon::Tag, "tag", "icons/tag.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Clock, "clock", "icons/clock.svg", kSvg_Clock, 304},
#else
    {Icon::Clock, "clock", "icons/clock.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Calendar, "calendar", "icons/calendar.svg", kSvg_Calendar, 410},
#else
    {Icon::Calendar, "calendar", "icons/calendar.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Link, "link", "icons/link.svg", kSvg_Link, 371},
#else
    {Icon::Link, "link", "icons/link.svg", nullptr, 0},
#endif
    {Icon::Unlink, "unlink", "icons/unlink.svg", nullptr, 0},
#if ICONLIB_ENABLE_EMBED
    {Icon::Web, "web", "icons/web.svg", kSvg_Web, 409},
#else
    {Icon::Web, "web", "icons/web.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Download, "download", "icons/download.svg", kSvg_Download, 370},
#else
    {Icon::Download, "download", "icons/download.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Upload, "upload", "icons/upload.svg", kSvg_Upload, 365},
#else
    {Icon::Upload, "upload", "icons/upload.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Cloud, "cloud", "icons/cloud.svg", kSvg_Cloud, 280},
#else
    {Icon::Cloud, "cloud", "icons/cloud.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Share, "share", "icons/share.svg", kSvg_Share, 364},
#else
    {Icon::Share, "share", "icons/share.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::External, "external", "icons/external.svg", kSvg_External, 388},
#else
    {Icon::External, "external", "icons/external.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Play, "play", "icons/play.svg", kSvg_Play, 263},
#else
    {Icon::Play, "play", "icons/play.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Pause, "pause", "icons/pause.svg", kSvg_Pause, 312},
#else
    {Icon::Pause, "pause", "icons/pause.svg", nullptr, 0},
#endif
    {Icon::Stop, "stop", "icons/stop.svg", nullptr, 0},
#if ICONLIB_ENABLE_EMBED
    {Icon::Prev, "prev", "icons/prev.svg", kSvg_Prev, 313},
#else
    {Icon::Prev, "prev", "icons/prev.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Next, "next", "icons/next.svg", kSvg_Next, 315},
#else
    {Icon::Next, "next", "icons/next.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Volume, "volume", "icons/volume.svg", kSvg_Volume, 280},
#else
    {Icon::Volume, "volume", "icons/volume.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::VolumeMute, "volume_mute", "icons/volume_mute.svg", kSvg_VolumeMute, 370},
#else
    {Icon::VolumeMute, "volume_mute", "icons/volume_mute.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Mic, "mic", "icons/mic.svg", kSvg_Mic, 418},
#else
    {Icon::Mic, "mic", "icons/mic.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Computer, "computer", "icons/computer.svg", kSvg_Computer, 370},
#else
    {Icon::Computer, "computer", "icons/computer.svg", nullptr, 0},
#endif
    {Icon::ControlPanel, "control_panel", "icons/control_panel.svg", nullptr, 0},
#if ICONLIB_ENABLE_EMBED
    {Icon::Cmd, "cmd", "icons/cmd.svg", kSvg_Cmd, 310},
#else
    {Icon::Cmd, "cmd", "icons/cmd.svg", nullptr, 0},
#endif
    {Icon::Registry, "registry", "icons/registry.svg", nullptr, 0},
#if ICONLIB_ENABLE_EMBED
    {Icon::Shutdown, "shutdown", "icons/shutdown.svg", kSvg_Shutdown, 308},
#else
    {Icon::Shutdown, "shutdown", "icons/shutdown.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Restart, "restart", "icons/restart.svg", kSvg_Restart, 321},
#else
    {Icon::Restart, "restart", "icons/restart.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Lock, "lock", "icons/lock.svg", kSvg_Lock, 321},
#else
    {Icon::Lock, "lock", "icons/lock.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Unlock, "unlock", "icons/unlock.svg", kSvg_Unlock, 322},
#else
    {Icon::Unlock, "unlock", "icons/unlock.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Key, "key", "icons/key.svg", kSvg_Key, 352},
#else
    {Icon::Key, "key", "icons/key.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Printer, "printer", "icons/printer.svg", kSvg_Printer, 407},
#else
    {Icon::Printer, "printer", "icons/printer.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Battery, "battery", "icons/battery.svg", kSvg_Battery, 326},
#else
    {Icon::Battery, "battery", "icons/battery.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Wifi, "wifi", "icons/wifi.svg", kSvg_Wifi, 401},
#else
    {Icon::Wifi, "wifi", "icons/wifi.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::App, "app", "icons/app.svg", kSvg_App, 462},
#else
    {Icon::App, "app", "icons/app.svg", nullptr, 0},
#endif
    {Icon::Group, "group", "icons/group.svg", nullptr, 0},
    {Icon::Tab, "tab", "icons/tab.svg", nullptr, 0},
#if ICONLIB_ENABLE_EMBED
    {Icon::Pin, "pin", "icons/pin.svg", kSvg_Pin, 480},
#else
    {Icon::Pin, "pin", "icons/pin.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Unpin, "unpin", "icons/unpin.svg", kSvg_Unpin, 204},
#else
    {Icon::Unpin, "unpin", "icons/unpin.svg", nullptr, 0},
#endif
    {Icon::Drag, "drag", "icons/drag.svg", nullptr, 0},
#if ICONLIB_ENABLE_EMBED
    {Icon::Plus, "plus", "icons/plus.svg", kSvg_Plus, 304},
#else
    {Icon::Plus, "plus", "icons/plus.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::Minus, "minus", "icons/minus.svg", kSvg_Minus, 261},
#else
    {Icon::Minus, "minus", "icons/minus.svg", nullptr, 0},
#endif
#if ICONLIB_ENABLE_EMBED
    {Icon::History, "history", "icons/history.svg", kSvg_History, 304},
#else
    {Icon::History, "history", "icons/history.svg", nullptr, 0},
#endif
    {Icon::Freq, "freq", "icons/freq.svg", nullptr, 0},
    {Icon::SearchHistory, "search_history", "icons/search_history.svg", nullptr, 0},
#if ICONLIB_ENABLE_EMBED
    {Icon::Clear, "clear", "icons/clear.svg", kSvg_Clear, 346},
#else
    {Icon::Clear, "clear", "icons/clear.svg", nullptr, 0},
#endif
};

std::size_t GetIconCount() {
    return sizeof(kIcons) / sizeof(kIcons[0]);
}

const IconAsset* GetAllIcons() {
    return kIcons;
}

const IconAsset* FindIcon(Icon icon) {
    if (icon == Icon::None) {
        return nullptr;
    }
    for (const auto& item : kIcons) {
        if (item.icon == icon) {
            return &item;
        }
    }
    return nullptr;
}

const IconAsset* FindIcon(const char* id) {
    if (id == nullptr) {
        return nullptr;
    }
    for (const auto& item : kIcons) {
        if (std::strcmp(item.id, id) == 0) {
            return &item;
        }
    }
    return nullptr;
}

const char* GetDynamicPath(Icon icon) {
    const auto* item = FindIcon(icon);
    if (item == nullptr) {
        return nullptr;
    }
    return item->fileName;
}

const char* GetDynamicPath(const char* id) {
    const auto* item = FindIcon(id);
    if (item == nullptr) {
        return nullptr;
    }
    return item->fileName;
}

const char* GetEmbeddedSvg(Icon icon) {
    const auto* item = FindIcon(icon);
    if (item == nullptr) {
        return nullptr;
    }
    return item->svg;
}

const char* GetEmbeddedSvg(const char* id) {
    const auto* item = FindIcon(id);
    if (item == nullptr) {
        return nullptr;
    }
    return item->svg;
}

} // namespace iconlib
