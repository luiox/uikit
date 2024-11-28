/*

词法分析（生成tokens）、语法分析（从tokens生成ast）、代码生成（遍历ast生成html）

 */

#ifndef MARKDOWN_HPP
#define MARKDOWN_HPP

#include <string>
#include <vector>
#include <memory>

namespace markdown {

///////////////////////////////////////////////////////////////////////////////

// 词法分析器
enum class TokenType
{
    Text, // 普通文本
    Title, // # text or ## text
    FortmattedText, // *text* or _text_
    Reference,  // > text
    Link, // [text](link)
    Image, // ![text](link)
    List, // + text or - text or 1. text
    CodeBlock, // ``` text``` or ```java xxx ````
    InlineCode, // `text`

    Unknown,
    End
};

struct Token{
    TokenType m_type;
    int m_line;
    int m_column;

    Token(TokenType type, int line, int column);

    ~Token() = default;
    virtual std::string toString() const = 0;
};

class TextToken : public Token{
public:
    TextToken(const std::string& text, int line, int column);
    std::string toString() const;
private:
    std::string m_text; 
};

class TitleToken : public Token{
public:
    TitleToken(int level, const std::string& text, int line, int column);
    std::string toString() const;
private:
    int m_level;
    std::string m_text;
};

class FortmattedTextToken : public Token{
public:
    FortmattedTextToken(const std::string& text, const std::string& style, int line, int column);
    std::string toString() const;
private:
    std::string m_text;
    std::string m_style;
};

class ReferenceToken : public Token{
public:
    ReferenceToken(const std::string& text , int line, int column);
    std::string toString() const;
private:
    std::string m_text;
};

class LinkToken : public Token{
public:
    LinkToken(const std::string& text, const std::string& link, int line, int column);
    std::string toString() const;
private:
    std::string m_text;
    std::string m_link;
};

class ImageToken : public Token{
public:
    ImageToken(const std::string& text, const std::string& link, int line, int column);
    std::string toString() const;
private:
    std::string m_text;
    std::string m_link;
};

class ListToken : public Token{
public:
    ListToken(const std::string& text, int line, int column);
    std::string toString() const;
private:
    std::vector<std::string> m_list;
    std::string m_style; // - or * or 1.
};

class CodeBlockToken : public Token{
public:
    CodeBlockToken(const std::string& code, const std::string& language, int line, int column);
    std::string toString() const;
private:
    std::string m_code;
    std::string m_language;
};

class InlineCodeToken : public Token{
public:
    InlineCodeToken(const std::string& code, int line, int column);
    std::string toString() const;
private:
    std::string m_code;
};

class UnknownToken : public Token{
public:
    UnknownToken(const std::string& text, int line, int column);
    std::string toString() const;
private:
    std::string m_text;
};

class Lexer{
public:
    Lexer(const std::string& text);
    Token* getNextToken();

    void tokenize();
    std::vector<Token*>& getTokens();
    
private:
    std::string m_text;
    int m_pos;
    int m_line;
    int m_column;
    std::vector<Token*> m_tokens;
};

///////////////////////////////////////////////////////////////////////////////

// 语法分析器

enum AstNodeType{
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


struct AstNode {
    AstNodeType type;
    std::string value;
    std::vector<std::shared_ptr<AstNode>> children;
    virtual ~AstNode() = default;
    virtual void accept(AstVisitor* visitor) = 0;
};

class TitleAstNode: public AstNode
{
public:
    TitleAstNode(int level);
    void accept(AstVisitor* visitor) override;  

private:
    int level;

};

class AstVisitor{
public:
    virtual void visit(const AstNode* node) = 0;
};

class Parser{
public:
    Parser(std::vector<Token*>& tokens);
    void parse();
    std::shared_ptr<AstNode> getAst();
private:
    std::shared_ptr<AstNode> ast;
};

///////////////////////////////////////////////////////////////////////////////

// 代码生成器

class HtmlCodeGenerator{
public:
    std::string generate(const std::shared_ptr<AstNode> ast);
};

}

#endif // !MARKDOWN_HPP
