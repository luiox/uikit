/*

词法分析（生成tokens）、语法分析（从tokens生成ast）、代码生成（遍历ast生成html）

 */

#ifndef MARKDOWN_HPP
#define MARKDOWN_HPP

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>


namespace markdown {

///////////////////////////////////////////////////////////////////////////////

// 词法分析器
enum class TokenType
{
    NewLine,            // 换行符
    Space,              // 空格
    Hash,               // #
    Asterisk,           // *
    Underscore,         // _
    GreaterThan,        // >
    LeftBracket,        // [
    RightBracket,       // ]
    LeftParenthesis,    // (
    RightParenthesis,   // )
    BackQuote,          // `
    Add,                // +
    Hyphen,             // -
    Dot,                // .
    Number,             // 0-9
    Text                // 普通文本
};

// 特殊字符映射
static std::map<char, TokenType> charMap = {
    {'\n', TokenType::NewLine},
    {' ', TokenType::Space},
    {'#', TokenType::Hash},
    {'*', TokenType::Asterisk},
    {'_', TokenType::Underscore},
    {'>', TokenType::GreaterThan},
    {'[', TokenType::LeftBracket},
    {']', TokenType::RightBracket},
    {'(', TokenType::LeftParenthesis},
    {')', TokenType::RightParenthesis},
    {'`', TokenType::BackQuote},
    {'+', TokenType::Add},
    {'-', TokenType::Hyphen},
    {'.', TokenType::Dot}
};

///////////////////////////////////////////////////////////////////////////////

// 语法分析器
enum class NodeKind
{
    Document,
    Paragraph,
    Header,
    List,
    ListItem,
    BlockQuote,
    CodeBlock,
};

struct Token
{
    TokenType   m_type;
    std::string m_value;
    int         m_line;
    int         m_column;

    Token(TokenType type, std::string m_value, int line, int column)
        : m_type(type)
        , m_value(m_value)
        , m_line(line)
        , m_column(column)
    {}
};

class Lexer
{
public:
    Lexer(const std::string& text);

    void tokenize();

    std::vector<Token>& getTokens();

private:
    char readChar();
    void unReadChar();

private:
    std::string        m_text;
    int                m_pos;
    int                m_line;
    int                m_column;
    std::vector<Token> m_tokens;
    std::string        m_buffer;
};

///////////////////////////////////////////////////////////////////////////////

// 语法分析器

enum AstNodeType
{
    Text,
    Title,
    Emphasis,
    Strong,
    Link,
    Image,
    List,
    CodeBlock,
    InlineCode,
    Unknown
};

class AstVisitor;


struct AstNode
{
    AstNodeType                           type;
    std::string                           value;
    std::vector<std::shared_ptr<AstNode>> children;
    virtual ~AstNode()                       = default;
    virtual void accept(AstVisitor* visitor) = 0;
};

class TitleAstNode : public AstNode
{
public:
    TitleAstNode(int level);
    void accept(AstVisitor* visitor) override;

private:
    int level;
};

class AstVisitor
{
public:
    virtual void visit(const AstNode* node) = 0;
};

class Parser
{
public:
    Parser(std::vector<Token*>& tokens);
    void                     parse();
    std::shared_ptr<AstNode> getAst();

private:
    std::shared_ptr<AstNode> ast;
};

///////////////////////////////////////////////////////////////////////////////

// 代码生成器

class HtmlCodeGenerator
{
public:
    std::string generate(const std::shared_ptr<AstNode> ast);
};

}   // namespace markdown

#endif   // !MARKDOWN_HPP
