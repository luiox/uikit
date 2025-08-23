mod config;

use tokio::io::{AsyncRead, AsyncWrite};
use config::ServerConfig;
use crate::config::file_exists;

fn main() {
    let config_file = "config.ini";
    let mut server_config = ServerConfig::default();
    // judge if config.ini exists
    if !file_exists(config_file) {
        server_config.write_config("config.ini");
    }else {
        server_config.read_config("config.ini");
    }
    println!("Server config: {:?}", server_config);

    // from tokio start tcp server
    let rt = tokio::runtime::Builder::new_current_thread()
        .enable_all()
        .build()
        .unwrap();
    rt.block_on(async {
        let listener = tokio::net::TcpListener::bind((server_config.host, server_config.port)).await.unwrap();
        loop{
            // let (mut socket, _) = listener.accept().await?;
            // tokio::spawn(async move {
            //     let mut buf = [0; 1024];
            //     loop{
            //         let n = match socket.read(&mut buf).await {
            //             Ok(n) if n == 0 => return,
            //             Ok(n) => n,
            //             Err(e) => {
            //                 eprint!("fail to read, err:{:?}",e);
            //                 return;
            //             }
            //         };
            //         if let Err(e) = socket.write_all(&buf[0..n]).await {
            //             eprint!("fail to write, err:{:?}",e);
            //             return;
            //         }
            //     }
            // });
        }

    });
}
