extern crate proc_macro;

use proc_macro::TokenStream;
use quote::quote;
use syn::{parse_macro_input, DeriveInput, Meta};

// #[proc_macro_derive(TMacro)]
// fn derive(input: TokenStream) -> TokenStream {
//     let ast = parse_macro_input!(input as DeriveInput);
//     let name = &ast.ident;
//
//     let gen_token = quote! {
//         impl MyTrait for #name {
//             fn foo(&self) {
//                 println!("Hello, world!");
//             }
//         }
//     };
//     gen_token.into()
// }

#[proc_macro_derive(AccessorGen, attributes(getter, setter))]
pub fn accessor_gen(input: TokenStream) -> TokenStream {
    let input = parse_macro_input!(input as DeriveInput);

    let name = &input.ident;
    let fields = if let syn::Data::Struct(syn::DataStruct { fields, .. }) = &input.data {
        fields
    } else {
        panic!("AccessorGen can only be used on structs");
    };

    let mut getters = Vec::new();
    let mut setters = Vec::new();

    for field in fields.iter() {
        let field_name = field.ident.as_ref().expect("Field must have a name");
        let field_type = &field.ty;

        // Check for getter attribute
        if has_attr(&field.attrs, "getter") {
            getters.push(quote! {
                pub fn #field_name(&self) -> &#field_type {
                    &self.#field_name
                }
            });
        }

        // Check for setter attribute
        if has_attr(&field.attrs, "setter") {
            setters.push(quote! {
                pub fn set_ #field_name(&mut self, value: #field_type) {
                    self.#field_name = value;
                }
            });
        }
    }

    let expanded = quote! {
        impl #name {
            #(#getters)*
            #(#setters)*
        }
    };

    TokenStream::from(expanded)
}

fn has_attr(attrs: &[syn::Attribute], attr_name: &str) -> bool {
    attrs.iter().any(|attr| {
        if let Ok(meta) = attr.parse_meta() {
            if let Meta::List(list) = meta {
                if list.path.is_ident(attr_name) {
                    return true;
                }
            }
        }
        false
    })
}
