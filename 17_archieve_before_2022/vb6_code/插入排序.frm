VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   4905
   ClientLeft      =   120
   ClientTop       =   465
   ClientWidth     =   6855
   LinkTopic       =   "Form1"
   ScaleHeight     =   4905
   ScaleWidth      =   6855
   StartUpPosition =   3  '窗口缺省
   Begin VB.CommandButton Command2 
      Caption         =   "降序排序"
      Height          =   495
      Left            =   2520
      TabIndex        =   3
      Top             =   1680
      Width           =   1215
   End
   Begin VB.CommandButton Command1 
      Caption         =   "升序排序"
      Height          =   495
      Left            =   2520
      TabIndex        =   2
      Top             =   720
      Width           =   1215
   End
   Begin VB.ListBox List2 
      Height          =   3120
      Left            =   4440
      TabIndex        =   1
      Top             =   480
      Width           =   1575
   End
   Begin VB.ListBox List1 
      Height          =   3120
      Left            =   480
      TabIndex        =   0
      Top             =   480
      Width           =   1575
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim a(1 To 15)
Dim n As Integer
Private Sub Form_Load()
'初始化随机数，并且生成15个数据
Randomize
n = 15
For i = 1 To n
    a(i) = Int(Rnd * 50 + 1)
    List1.AddItem (CStr(a(i)))
Next i

End Sub
Private Sub Command1_Click()
'插入排序升序
For i = 2 To n
    k = a(i)
    For j = i - 1 To 1 Step -1
        If a(j) <= k Then Exit For
            a(j + 1) = a(j)
    Next j
    a(j + 1) = k
Next i
'输出
List2.Clear
For i = 1 To n
    List2.AddItem (CStr(a(i)))
Next i

End Sub
Private Sub Command2_Click()
'插入排序降序
For i = 2 To n
    k = a(i)
    For j = i - 1 To 1 Step -1
        If a(j) >= k Then Exit For
            a(j + 1) = a(j)
    Next j
    a(j + 1) = k
Next i

'输出
List2.Clear
For i = 1 To n
    List2.AddItem (CStr(a(i)))
Next i
End Sub
