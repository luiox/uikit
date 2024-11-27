#ifndef MARKDOWN_HPP
#define MARKDOWN_HPP

#include <string>
#include <vector>
#include <memory>

namespace markdown {

enum class TokenType{
    Text,
    WhiteSpace,
    NewLine,
    Title,
    TitleEnd,
    EmphasisStart,
    EmphasisEnd,
    StrongStart,
    StrongEnd,
    LinkStart,
    LinkEnd,
    LinkContent,
    ImageStart,
    ImageEnd,
    ImageContent,
    ListStart,
    ListEnd,
    ListContent,
    CodeBlockStart,
    CodeBlockEnd,
    CodeBlockContent,
    InlineCodeStart,
    InlineCodeEnd,
    InlineCodeContent,

    Unknown,
    End
};

struct Token{
    TokenType type;
    std::string value;
    int line;
    int column;
};

class Lexer{
public:
    Lexer(const std::string& text);
    void tokenize();
    std::vector<Token>& getTokens();
    Token getNextToken();
private:
    std::string text;
    int pos;
    int line;
    int column;
    std::vector<Token> tokens;
};

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

struct AstNode{
    AstNodeType type;
    std::string value;
    std::vector<std::shared_ptr<AstNode>> children;
};

class Parser{
public:
    Parser(std::vector<Token>& tokens);
    void parse();
    std::shared_ptr<AstNode> getAst();
private:
    std::shared_ptr<AstNode> ast;
};

class HtmlCodeGenerator{
public:
    std::string generate(const std::shared_ptr<AstNode> ast);
};

}

#endif // !MARKDOWN_HPP
