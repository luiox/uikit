target("ant_fast_gaussian_blur")
    set_kind("headeronly")
    set_languages("cxx17")
    set_default(false)

    add_includedirs(".", {public = true})
    add_headerfiles("fast_gaussian_blur_template.h")
