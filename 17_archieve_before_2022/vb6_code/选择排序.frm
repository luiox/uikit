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
   StartUpPosition =   3  '´°¿ÚÈ±Ê¡
   Begin VB.CommandButton Command2 
      Caption         =   "½µÐòÅÅÐò"
      Height          =   495
      Left            =   2520
      TabIndex        =   3
      Top             =   1680
      Width           =   1215
   End
   Begin VB.CommandButton Command1 
      Caption         =   "ÉýÐòÅÅÐò"
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
'³õÊ¼»¯Ëæ»úÊý£¬²¢ÇÒÉú³É15¸öÊý¾Ý
Randomize
n = 15
For i = 1 To n
    a(i) = Int(Rnd * 50 + 1)
    List1.AddItem (CStr(a(i)))
Next i

End Sub
Private Sub Command1_Click()
'Ã°ÅÝÅÅÐòÉýÐò
For i = 1 To n - 1
    For j = n To i + 1 Step -1
        If a(j) < a(j - 1) Then
            t = a(j): a(j) = a(j - 1): a(j - 1) = t
        End If
    Next j
Next i
'Êä³ö
List2.Clear
For i = 1 To n
    List2.AddItem (CStr(a(i)))
Next i

End Sub
Private Sub Command2_Click()
'Ã°ÅÝÅÅÐò½µÐò
For i = 1 To n - 1
    For j = n To i + 1 Step -1
        If a(j) > a(j - 1) Then
            t = a(j): a(j) = a(j - 1): a(j - 1) = t
        End If
    Next j
Next i

'Êä³ö
List2.Clear
For i = 1 To n
    List2.AddItem (CStr(a(i)))
Next i
End Sub
