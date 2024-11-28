#include "Markdown.hpp"
#include <sstream>

namespace markdown {

Token::Token(TokenType type, int line, int column)
    : m_type(type)
    , m_line(line)
    , m_column(column)
{}

TextToken::TextToken(const std::string& text, int line, int column)
    : Token(TokenType::Text, line, column)
    , m_text(text)
{}

std::string TextToken::toString() const
{
    return "type:TextToken, m_text:" + m_text;
}

TitleToken::TitleToken(int level, const std::string& text, int line, int column)
    : Token(TokenType::Title, line, column)
    , m_level(level)
    , m_text(text)
{}

std::string TitleToken::toString() const
{
    return "type:TitleToken, m_level:" + std::to_string(m_level) + ", m_text:" + m_text;
}

FortmattedTextToken::FortmattedTextToken(const std::string& text, const std::string& style,
                                         int line, int column)
    : Token(TokenType::FortmattedText, line, column)
    , m_style(style)
    , m_text(text)
{}

std::string FortmattedTextToken::toString() const
{
    return "type:FortmattedTextToken, m_text:" + m_text;
}

ReferenceToken::ReferenceToken(const std::string& text, int line, int column)
    : Token(TokenType::Reference, line, column)
    , m_text(text)
{}

std::string ReferenceToken::toString() const
{
    return "type:ReferenceToken, m_text:" + m_text;
}

LinkToken::LinkToken(const std::string& text, const std::string& link, int line, int column)
    : Token(TokenType::Link, line, column)
    , m_link(link)
    , m_text(text)
{}

std::string LinkToken::toString() const
{
    return "type:LinkToken, m_text:" + m_text;
}

ImageToken::ImageToken(const std::string& text, const std::string& link, int line, int column)
    : Token(TokenType::Image, line, column)
    , m_link(link)
    , m_text(text)
{}

std::string ImageToken::toString() const
{
    return "type:ImageToken, m_text:" + m_text;
}

ListToken::ListToken(const std::string& text, int line, int column)
    : Token(TokenType::List, line, column)
{}

std::string ListToken::toString() const
{
    std::stringstream ss;
    int               idx = 0;
    for (auto& item : m_list) {
        ss << idx << item << std::endl;
        idx++;
    }
    return ss.str();
}

CodeBlockToken::CodeBlockToken(const std::string& code, const std::string& language, int line,
                               int column)
    : Token(TokenType::CodeBlock, line, column)
    , m_code(code)
    , m_language(language)
{}

std::string CodeBlockToken::toString() const
{
    return "type:CodeBlockToken, m_text:";
}

InlineCodeToken::InlineCodeToken(const std::string& code, int line, int column)
    : Token(TokenType::InlineCode, line, column)
    , m_code(code)
{}

std::string InlineCodeToken::toString() const
{
    return "type:InlineCodeToken, m_text:";
}

UnknownToken::UnknownToken(const std::string& text, int line, int column)
    : Token(TokenType::Unknown, line, column)
{}

std::string UnknownToken::toString() const
{
    return "type:UnknownToken, m_text:";
}

Lexer::Lexer(const std::string& text)
    : m_text(text)
    , m_pos(0)
    , m_column(0)
    , m_line(0)
{
    m_tokens.clear();
}

// 获取下一个token
Token* Lexer::getNextToken()
{
    // 先读一个
    if (m_text[m_pos] == '#') {
        // 如果是#，则读取一个或多个#，直到不是#为止
        int count = 0;
        while (m_text[m_pos] == '#') {
            count++;
            m_pos++;
        }
        // 如果后面是空格，则说明是标题
        if (m_text[m_pos] == ' ') {
            m_pos++;
            std::string title;
            // 往后查找到换行
            while (m_text[m_pos] != '\n') {
                title += m_text[m_pos];
                m_pos++;
            }
            return new TitleToken(count, title, m_line, m_column);
        }
    }
    if (m_text[m_pos] == '>') {
        // 引用
        m_pos++;
        std::string reference;
        // 往后查找到换行
        while (m_text[m_pos] != '\n') {
            reference += m_text[m_pos];
            m_pos++;
        }
        return new ReferenceToken(reference, m_line, m_column);
    }
    if (m_text[m_pos] == '!') {
        m_pos++;
        if (m_text[m_pos] == '[') {
            m_pos++;
            std::string link;
            // 往后查找到换行
            while (m_text[m_pos] != ']') {}
            // 往后看一个是否有()


            // 如果不是就退回去


            // 图片
        }
    }
    if (m_text[m_pos] == '[') {
        // 链接
        m_pos++;
        // 查找看是否有']'和()
    }
    if (m_text[m_pos] == '`') {
        // 代码
        m_pos++;

        // 往后看一个是否是```

        // 如果不是就看看是不是`code`这种类型

        // 再往后找有没有```结束

        // 如果不是就退回去
    }
    if (m_text[m_pos] == '+' || m_text[m_pos] == '-') {
        // 列表
        m_pos++;
        int count = 1;
        while (m_text[m_pos] == ' ') {
            m_pos++;
            count++;
        }
        std::string title;
        while (m_text[m_pos] != '\n') {
            title += m_text[m_pos];
            m_pos++;
        }
    }

    return nullptr;
}

void Lexer::tokenize()
{
    // 只要没有到字符串末尾，就不断获取下一个token
    while (m_pos < m_text.length()) {
        Token* token = getNextToken();
        if (token == nullptr) {
            break;
        }
        // 将token添加到token列表中
        m_tokens.push_back(token);
    }
}

std::vector<Token*>& Lexer::getTokens()
{
    return m_tokens;
}

///////////////////////////////////////////////////////////////////////////////

void TitleAstNode::accept(AstVisitor* visitor)
{
    visitor->visit(this);
}

std::string HtmlCodeGenerator::generate(const std::shared_ptr<AstNode> ast)
{

    return "";
}

}   // namespace markdown