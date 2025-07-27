using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CaKeilPlugin
{
    internal static class FileEncodeUtil
    {
        // Token: 0x06000001 RID: 1 RVA: 0x00002050 File Offset: 0x00000250
        public static Encoding GetType(string aFile)
        {
            Encoding type;
            using (FileStream fileStream = new FileStream(aFile, FileMode.Open, FileAccess.Read))
            {
                type = FileEncodeUtil.GetType(fileStream);
            }
            return type;
        }

        // Token: 0x06000002 RID: 2 RVA: 0x00002090 File Offset: 0x00000290
        private static Encoding GetType(FileStream fs)
        {
            byte[] expr_07 = new byte[]
            {
                255,
                254,
                65
            };
            byte[] expr_19 = new byte[3];
            expr_19[0] = 254;
            expr_19[1] = 255;
            byte[] expr_30 = new byte[]
            {
                239,
                187,
                191
            };
            Encoding result = Encoding.Default;
            BinaryReader binaryReader = new BinaryReader(fs, Encoding.Default);
            int count;
            int.TryParse(fs.Length.ToString(), out count);
            byte[] array = binaryReader.ReadBytes(count);
            bool flag = FileEncodeUtil.IsUTF8Bytes(array) || (array[0] == 239 && array[1] == 187 && array[2] == 191);
            if (flag)
            {
                result = Encoding.UTF8;
            }
            else
            {
                bool flag2 = array[0] == 254 && array[1] == 255 && array[2] == 0;
                if (flag2)
                {
                    result = Encoding.BigEndianUnicode;
                }
                else
                {
                    bool flag3 = array[0] == 255 && array[1] == 254 && array[2] == 65;
                    if (flag3)
                    {
                        result = Encoding.Unicode;
                    }
                }
            }
            binaryReader.Close();
            return result;
        }

        // Token: 0x06000003 RID: 3 RVA: 0x000021BC File Offset: 0x000003BC
        private static bool IsUTF8Bytes(byte[] data)
        {
            int num = 1;
            bool result;
            for (int i = 0; i < data.Length; i++)
            {
                byte b = data[i];
                bool flag = num == 1;
                if (flag)
                {
                    bool flag2 = b >= 128;
                    if (flag2)
                    {
                        while (((b = (byte)(b << 1)) & 128) > 0)
                        {
                            num++;
                        }
                        bool flag3 = num == 1 || num > 6;
                        if (flag3)
                        {
                            result = false;
                            return result;
                        }
                    }
                }
                else
                {
                    bool flag4 = (b & 192) != 128;
                    if (flag4)
                    {
                        result = false;
                        return result;
                    }
                    num--;
                }
            }
            bool flag5 = num > 1;
            if (flag5)
            {
                throw new Exception("非预期的byte格式");
            }
            result = true;
            return result;
        }
    }
}
