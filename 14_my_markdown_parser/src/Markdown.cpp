#include "Markdown.hpp"

#include <cctype>
#include <sstream>

namespace markdown {


Lexer::Lexer(const std::string& text)
    : m_text(text)
    , m_pos(0)
    , m_column(0)
    , m_line(1)
{
    m_tokens.clear();
}

char Lexer::readChar()
{
    if (m_pos < m_text.size()) {
        char c = m_text[m_pos];
        m_pos++;
        m_column++;
        m_buffer += c;
        return c;
    }
    return 0;
}
void Lexer::unReadChar()
{
    if (m_pos > 0) {
        m_pos--;
        m_column--;
        m_buffer.pop_back();
    }
}

void Lexer::tokenize()
{
    while(m_pos < m_text.size()) {
        auto ch = m_text[m_pos];
        m_pos++;
        m_column++;

        if (charMap.find(ch) != charMap.end()) {
            if (m_buffer.empty()) {
                m_tokens.push_back(Token(charMap[ch], m_buffer, m_line, m_column));
                m_buffer.clear();
            }
        }
        else if (std::isdigit(ch)) {
            m_buffer += ch;
        }
        else {
            
        }
    }
}

std::vector<Token>& Lexer::getTokens()
{
    return m_tokens;
}

///////////////////////////////////////////////////////////////////////////////

void TitleAstNode::accept(AstVisitor* visitor)
{
    visitor->visit(this);
}

///////////////////////////////////////////////////////////////////////////////

std::string HtmlCodeGenerator::generate(const std::shared_ptr<AstNode> ast)
{

    return "";
}

}   // namespace markdown