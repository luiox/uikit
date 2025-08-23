// See https://aka.ms/new-console-template for more information

using System.Runtime.InteropServices;
using System.Text;

namespace CaKeilPlugin
{
    public class Program
    {
        #if DEBUG
                // 如果是Debug模式，不隐藏控制台窗口
        #else
        // 如果是Release模式，隐藏控制台窗口
        [DllImport("kernel32.dll")]
        static extern IntPtr GetConsoleWindow();

        [DllImport("user32.dll")]
        static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        const int SW_HIDE = 0;
        const int SW_SHOW = 5;
        #endif

        private static void PrintUsage()
        {
            var logo = """
                  _____           _  __         _   _   _____    _                   _         
                 / ____|         | |/ /        (_) | | |  __ \  | |                 (_)        
                | |        __ _  | ' /    ___   _  | | | |__) | | |  _   _    __ _   _   _ __  
                | |       / _` | |  <    / _ \ | | | | |  ___/  | | | | | |  / _` | | | | '_ \ 
                | |____  | (_| | | . \  |  __/ | | | | | |      | | | |_| | | (_| | | | | | | |
                 \_____|  \__,_| |_|\_\  \___| |_| |_| |_|      |_|  \__,_|  \__, | |_| |_| |_|
                                                                              __/ |            
                                                                             |___/             
                """;
            var version = "0.1.0";
            Console.WriteLine(logo);
            Console.WriteLine($"CaKeilPlugin version is :{version}");
            Console.WriteLine("CaKeilPlugin Usage:");
            Console.WriteLine("  -<n> 0.reset the config");
            Console.WriteLine("       1.print help infomation");
            Console.WriteLine("       2.file comment");
            Console.WriteLine("       3.function comment");
            Console.WriteLine("       4.licence comment");
        }

        private static string GetDefineStr(string aFileName)
        {
            string fileNameWithoutExtension = Path.GetFileNameWithoutExtension(aFileName);
            StringBuilder stringBuilder = new StringBuilder();
            stringBuilder.Append("_");
            string text = fileNameWithoutExtension;
            for (int i = 0; i < text.Length; i++)
            {
                char c = text[i];
                bool flag = c >= 'A' && c <= 'Z';
                if (flag)
                {
                    stringBuilder.Append("_").Append(c);
                }
                else
                {
                    stringBuilder.Append(c.ToString().ToUpper());
                }
            }
            string value = aFileName.Substring(aFileName.LastIndexOf('.') + 1).ToUpper();
            stringBuilder.Append("_").Append(value).Append("_");
            bool flag2 = stringBuilder[0] == '_' && stringBuilder[1] != '_';
            if (flag2)
            {
                stringBuilder.Insert(0, "_");
            }
            return stringBuilder.ToString();
        }

        // Token: 0x06000005 RID: 5 RVA: 0x000022D4 File Offset: 0x000004D4
        private static void InsertComments(string aFile)
        {
            string fileName = Path.GetFileName(aFile);
            string text = Path.GetExtension(aFile).Equals(".h", StringComparison.CurrentCultureIgnoreCase) ? "File.h" : "File.c";
            text = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, text);
            bool flag = !File.Exists(text);
            if (flag)
            {
                Console.WriteLine(string.Format("模板文件[{0}]未找到...", text));
            }
            else
            {
                string text2 = File.ReadAllText(text, FileEncodeUtil.GetType(text));
                text2 = Program.TemplateDeal(text2, fileName);
                Encoding type = FileEncodeUtil.GetType(aFile);
                string text3 = File.ReadAllText(aFile, type);
                text3 = text2 + text3;
                File.WriteAllText(aFile, text3, type);
            }
        }

        public static void Main(string[] args)
        {
            #if !DEBUG
                // 隐藏控制台窗口
                IntPtr consoleHandle = GetConsoleWindow();
                ShowWindow(consoleHandle, SW_HIDE);
            #endif
            if (args == null || args.Length < 1)
            {
                PrintUsage();
            }
            else
            {
                string text = args[0];
                if (text == "-0")
                {
                    Console.WriteLine("reset the config");
                    //ConfigUtil.ResetConfig();
                    return;
                }
                else if(text == "-1")
                {
                    Console.WriteLine("print help infomation");
                }
                else if (text == "-2")
                {
                    var file = args[1];
                    Console.WriteLine("file comment");
                    bool flag2 = !File.Exists(file);
                    Console.WriteLine($"text = {file}");
                    if (flag2)
                    {
                        Console.WriteLine("未能获取到文件路径, 不支持新建未保存的文件...");
                    }
                    else
                    {
                        Program.InsertComments(file);
                    }
                }
                // else if (text == "-3")
            }
            
        }
        private static string TemplateDeal(string aTemplate, string aFileName)
        {
            aTemplate = aTemplate.Replace("{FileName}", aFileName);
            aTemplate = aTemplate.Replace("{Year}", DateTime.Now.Year.ToString());
            aTemplate = aTemplate.Replace("{Date}", DateTime.Now.ToString("yyyy-MM-dd"));
            string defineStr = Program.GetDefineStr(aFileName);
            StringBuilder stringBuilder = new StringBuilder();
            stringBuilder.Append("#ifndef ").AppendLine(defineStr);
            stringBuilder.Append("#define ").AppendLine(defineStr).AppendLine();
            stringBuilder.AppendLine().Append("#endif").AppendLine();
            aTemplate = aTemplate.Replace("{ifndef}", stringBuilder.ToString());
            return aTemplate;
        }
    }
}

