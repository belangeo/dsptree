#!/usr/bin/env python
# encoding: utf-8
import wx, os, sys, unicodedata
from types import UnicodeType
import wx.stc as stc

def ensureNFD(unistr):
    if sys.platform in ['linux2', 'win32']:
        encodings = [sys.getdefaultencoding(), sys.getfilesystemencoding(),
                     'cp1252', 'iso-8859-1', 'utf-16']
        format = 'NFC'
    else:
        encodings = [sys.getdefaultencoding(), sys.getfilesystemencoding(),
                     'macroman', 'iso-8859-1', 'utf-16']
        format = 'NFC'
    decstr = unistr
    if type(decstr) != UnicodeType:
        for encoding in encodings:
            try:
                decstr = decstr.decode(encoding)
                break
            except UnicodeDecodeError:
                continue
            except:
                decstr = "UnableToDecodeString"
                print "Unicode encoding not in a recognized format..."
                break
    if decstr == "UnableToDecodeString":
        return unistr
    else:
        return unicodedata.normalize(format, decstr)

class ExprLexer(object):
    """Defines simple interface for custom lexer objects."""
    STC_EXPR_DEFAULT, STC_EXPR_KEYWORD, STC_EXPR_KEYWORD2, STC_EXPR_COMMENT, \
    STC_EXPR_VARIABLE, STC_EXPR_LETVARIABLE = range(6)
    def __init__(self):
        super(ExprLexer, self).__init__()
        
        self.alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~"
        self.digits = "0123456789"
        self.keywords = ["sin", "cos", "tan", "tanh", "atan", "atan2", "sqrt", "log",
                         "log2", "log10", "pow", "abs", "floor", "ceil", "exp", "round",
                         "min", "max", "randf", "randi", "sah", "count", "pi", "twopi",
                         "e", "if", "rpole", "rzero", "neg", "and", "or", "wrap"]
        self.keywords2 = ["def", "load", "var", "let", "let~", "const", "global", "buffer"]

    def StyleText(self, evt):
        """Handle the EVT_STC_STYLENEEDED event."""
        stc = evt.GetEventObject()
        last_styled_pos = stc.GetEndStyled()
        line = stc.LineFromPosition(last_styled_pos)
        start_pos = stc.PositionFromLine(line)
        end_pos = evt.GetPosition()

        while start_pos < end_pos:
            stc.StartStyling(start_pos, 0x1f)
            curchar = chr(stc.GetCharAt(start_pos))
            if curchar in self.alpha:
                start = stc.WordStartPosition(start_pos, True)
                end = start + 1
                while chr(stc.GetCharAt(end)) != " " and end < stc.GetLength():
                    end += 1
                word = stc.GetTextRange(start, end)
                if word in self.keywords:
                    style = self.STC_EXPR_KEYWORD
                    stc.SetStyling(len(word), style)
                elif word in self.keywords2:
                    style = self.STC_EXPR_KEYWORD2
                    stc.SetStyling(len(word), style)
                else:
                    style = self.STC_EXPR_DEFAULT
                    stc.SetStyling(len(word), style)
                start_pos += len(word)
            elif curchar == ";":
                eol = stc.GetLineEndPosition(stc.LineFromPosition(start_pos))
                style = self.STC_EXPR_COMMENT
                stc.SetStyling(eol-start_pos, style)
                start_pos = eol
            else:
                style = self.STC_EXPR_DEFAULT
                stc.SetStyling(1, style)
                start_pos += 1

class ExprEditor(stc.StyledTextCtrl):    
    def __init__(self, parent, id=-1):
        stc.StyledTextCtrl.__init__(self, parent, id)

        if sys.platform == "darwin":
            accel_ctrl = wx.ACCEL_CMD
            self.faces = {'mono' : 'Monospace', 'size' : 14}
        else:
            accel_ctrl = wx.ACCEL_CTRL
            self.faces = {'mono' : 'Monospace', 'size' : 9}

        atable = wx.AcceleratorTable([(accel_ctrl,  wx.WXK_RETURN, 10000),
                                      (accel_ctrl,  ord("z"), wx.ID_UNDO),
                                      (accel_ctrl|wx.ACCEL_SHIFT,  ord("z"), wx.ID_REDO)])
        self.SetAcceleratorTable(atable)
        
        self.Bind(wx.EVT_MENU, self.onExecute, id=10000)
        self.Bind(wx.EVT_MENU, self.undo, id=wx.ID_UNDO)
        self.Bind(wx.EVT_MENU, self.redo, id=wx.ID_REDO)
        self.Bind(stc.EVT_STC_UPDATEUI, self.OnUpdateUI)

        self.lexer = ExprLexer()
        
        self.currentfile = ""
        self.modified = False

        self.setup()
        self.setCmdKeys()
        self.setStyle()

    def undo(self, evt):
        self.Undo()

    def redo(self, evt):
        self.Redo()

    def setup(self):
        self.SetUseAntiAliasing(True)
        self.SetIndent(2)
        self.SetBackSpaceUnIndents(True)
        self.SetTabIndents(True)
        self.SetTabWidth(2)
        self.SetUseTabs(False)
        self.SetMargins(2, 2)
        self.SetMarginWidth(1, 1)

    def setCmdKeys(self):
        self.CmdKeyAssign(ord('='), stc.STC_SCMOD_CTRL, stc.STC_CMD_ZOOMIN)
        self.CmdKeyAssign(ord('-'), stc.STC_SCMOD_CTRL, stc.STC_CMD_ZOOMOUT)

    def setStyle(self):
        self.SetLexer(wx.stc.STC_LEX_CONTAINER)
        self.SetStyleBits(5)
        self.Bind(wx.stc.EVT_STC_STYLENEEDED, self.OnStyling)

        self.SetCaretForeground("#000000")
        self.SetCaretWidth(2)
        # Global default styles for all languages
        self.StyleSetSpec(stc.STC_STYLE_DEFAULT, "face:%(mono)s,size:%(size)d" % self.faces)
        self.StyleClearAll()

        self.StyleSetSpec(stc.STC_STYLE_DEFAULT, "face:%(mono)s,size:%(size)d" % self.faces)
        self.StyleSetSpec(stc.STC_STYLE_CONTROLCHAR, "face:%(mono)s" % self.faces)
        self.StyleSetSpec(stc.STC_STYLE_BRACELIGHT, "fore:#FFFFFF,back:#0000FF,bold")
        self.StyleSetSpec(stc.STC_STYLE_BRACEBAD, "fore:#000000,back:#FF0000,bold")

        # Expr specific styles
        self.StyleSetSpec(self.lexer.STC_EXPR_DEFAULT, "fore:#000000,face:%(mono)s,size:%(size)d" % self.faces)
        self.StyleSetSpec(self.lexer.STC_EXPR_KEYWORD, "fore:#3300DD,face:%(mono)s,size:%(size)d,bold" % self.faces)
        self.StyleSetSpec(self.lexer.STC_EXPR_KEYWORD2, "fore:#0033FF,face:%(mono)s,size:%(size)d,bold" % self.faces)
        self.StyleSetSpec(self.lexer.STC_EXPR_VARIABLE, "fore:#006600,face:%(mono)s,size:%(size)d,bold" % self.faces)
        self.StyleSetSpec(self.lexer.STC_EXPR_LETVARIABLE, "fore:#555500,face:%(mono)s,size:%(size)d,bold" % self.faces)
        self.StyleSetSpec(self.lexer.STC_EXPR_COMMENT, "fore:#444444,face:%(mono)s,size:%(size)d,italic" % self.faces)

        self.SetSelBackground(1, "#CCCCDD")

    def OnStyling(self, evt):
        self.lexer.StyleText(evt)

    def loadfile(self, filename):
        self.LoadFile(filename)
        self.currentfile = filename
        self.GetParent().SetTitle(self.currentfile)

    def savefile(self, filename):
        self.currentfile = filename
        self.GetParent().SetTitle(self.currentfile)
        self.SaveFile(filename)
        self.OnUpdateUI(None)

    def OnUpdateUI(self, evt):
        # check for matching braces
        braceAtCaret = -1
        braceOpposite = -1
        charBefore = None
        caretPos = self.GetCurrentPos()

        if caretPos > 0:
            charBefore = self.GetCharAt(caretPos - 1)
            styleBefore = self.GetStyleAt(caretPos - 1)

        # check before
        if charBefore and chr(charBefore) in "[]{}()":
            braceAtCaret = caretPos - 1

        # check after
        if braceAtCaret < 0:
            charAfter = self.GetCharAt(caretPos)
            styleAfter = self.GetStyleAt(caretPos)

            if charAfter and chr(charAfter) in "[]{}()":
                braceAtCaret = caretPos
        if braceAtCaret >= 0:
            braceOpposite = self.BraceMatch(braceAtCaret)

        if braceAtCaret != -1  and braceOpposite == -1:
            self.BraceBadLight(braceAtCaret)
        else:
            self.BraceHighlight(braceAtCaret, braceOpposite)
        # Check if horizontal scrollbar is needed
        self.checkScrollbar()

    def checkScrollbar(self):
        lineslength = [self.LineLength(i)+1 for i in range(self.GetLineCount())]
        maxlength = max(lineslength)
        width = self.GetCharWidth() + (self.GetZoom() * 0.5)
        if (self.GetSize()[0]) < (maxlength * width):
            self.SetUseHorizontalScrollBar(True)
        else:
            self.SetUseHorizontalScrollBar(False)

    def onExecute(self, evt):
        pos = self.GetCurrentPos()
        self.SetCurrentPos(pos)
        self.SetSelection(pos, pos)

class ExprEditorFrame(wx.Frame):
    def __init__(self, parent=None):
        wx.Frame.__init__(self, parent, pos=(50,50), size=(800,650))
        self.editor = ExprEditor(self, -1)
        self.menubar = wx.MenuBar()
        self.fileMenu = wx.Menu()
        self.fileMenu.Append(wx.ID_OPEN, "Open\tCtrl+O")
        self.Bind(wx.EVT_MENU, self.open, id=wx.ID_OPEN)
        self.fileMenu.AppendSeparator()
        self.fileMenu.Append(wx.ID_SAVE, "Save\tCtrl+S")
        self.Bind(wx.EVT_MENU, self.save, id=wx.ID_SAVE)
        self.fileMenu.Append(wx.ID_SAVEAS, "Save As...\tShift+Ctrl+S")
        self.Bind(wx.EVT_MENU, self.saveas, id=wx.ID_SAVEAS)
        self.fileMenu.AppendSeparator()
        self.fileMenu.Append(wx.ID_EXIT, 'Close\tCtrl+Q', kind=wx.ITEM_NORMAL)
        self.Bind(wx.EVT_MENU, self.close, id=wx.ID_EXIT)
        self.menubar.Append(self.fileMenu, "&File")
        self.SetMenuBar(self.menubar)

    def open(self, evt):
        dlg = wx.FileDialog(self, message="Choose a file", 
                            defaultDir=os.path.expanduser("~"),
                            defaultFile="", style=wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            path = ensureNFD(dlg.GetPath())
            self.editor.loadfile(path)
        dlg.Destroy()

    def close(self, evt):
        self.Destroy()

    def save(self, evt):
        path = self.editor.currentfile
        if not path:
            self.saveas(None)
        else:
            self.editor.savefile(path)

    def saveas(self, evt):
        deffile = os.path.split(self.editor.currentfile)[1]
        dlg = wx.FileDialog(self, message="Save file as ...", 
                            defaultDir=os.path.expanduser("~"),
                            defaultFile=deffile, style=wx.SAVE)
        dlg.SetFilterIndex(0)
        if dlg.ShowModal() == wx.ID_OK:
            path = ensureNFD(dlg.GetPath())
            self.editor.savefile(path)
        dlg.Destroy()

    def update(self, text):
        self.editor.SetText(text)

if __name__ == "__main__":
    app = wx.App(False)
    mainFrame = ExprEditorFrame()
    if len(sys.argv) > 1:
        mainFrame.editor.loadfile(sys.argv[1])
    mainFrame.Show()
    app.MainLoop()
