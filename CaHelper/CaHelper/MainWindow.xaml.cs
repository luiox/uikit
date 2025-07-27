using System.IO;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using IOPath = System.IO.Path;

namespace CaHelper
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            var headerFileName = header_file_name_edit.Text;
            var sourceFileName = source_file_name_edit.Text;

            // 获取当前程序的运行路径
            string currentDirectory = AppDomain.CurrentDomain.BaseDirectory;

            // 创建文件的完整路径
            string headerFilePath = IOPath.Combine(currentDirectory, $"{headerFileName}");
            string sourceFilePath = IOPath.Combine(currentDirectory, $"{sourceFileName}");

            // 如果该文件已经存在，则删除它
            if (File.Exists(headerFilePath))
            {
                File.Delete(headerFilePath);
            }

            // 创建新的头文件，并写入一些内容
            using (StreamWriter sw = File.CreateText(headerFilePath))
            {
                var fileNameWithoutExtension = IOPath.GetFileNameWithoutExtension(headerFileName);
                sw.WriteLine($"#ifndef {fileNameWithoutExtension.ToUpper()}_H");
                sw.WriteLine($"#define {fileNameWithoutExtension.ToUpper()}_H ");
                sw.WriteLine("");
                sw.WriteLine("");
                sw.WriteLine("");
                sw.WriteLine($"#endif // !{fileNameWithoutExtension.ToUpper()}_H");
                sw.WriteLine("");
            }

            // 如果该文件已经存在，则删除它
            if (File.Exists(sourceFilePath))
            {
                File.Delete(sourceFilePath);
            }

            // 创建新的源文件，并写入一些内容
            using (StreamWriter sw = File.CreateText(sourceFilePath))
            {
                if(use_angle_brackets_btn.IsChecked == true)
                {
                    sw.WriteLine($"#include <{headerFileName}>");
                }
                else if(use_double_quotation_marks_btn.IsChecked == true)
                {
                    sw.WriteLine($"#include \"{headerFileName}\"");
                }
                //sw.WriteLine("");
            }
        }

        private void header_file_name_edit_TextChanged(object sender, TextChangedEventArgs e)
        {
            if(enable_same_check_box.IsChecked == true)
            {
                if (header_file_name_edit.Text.Contains(".h")) {
                    source_file_name_edit.Text = IOPath.GetFileNameWithoutExtension(header_file_name_edit.Text)+ ".c";
                    return;
                }
                source_file_name_edit.Text = header_file_name_edit.Text;
            }
        }
    }
}