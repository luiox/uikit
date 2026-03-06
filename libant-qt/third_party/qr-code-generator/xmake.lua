target("ant_qrcodegen")
    set_kind("static")
    set_languages("cxx17")
    set_default(false)

    add_includedirs(".", {public = true})
    add_headerfiles("qrcodegen.hpp")
    add_files("qrcodegen.cpp")
