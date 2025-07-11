use std::path::Path;
use ini::configparser::ini::Ini;
use ini::ini;

#[derive(Debug)]
pub struct ServerConfig{
    pub port: u16,
    pub host: String,
}

pub fn file_exists(file_path: &str) -> bool{
    Path::new(file_path).exists()
}

impl ServerConfig{

    pub fn default() -> Self{
        ServerConfig{
            port: 8080,
            host: "0.0.0.0".to_string(),
        }
    }

    pub fn new(port: u16, host: String) -> Self{
        ServerConfig{
            port,
            host,
        }
    }

    pub fn read_config(&mut self, config_file: &str) {
        let map = ini!(config_file);
        self.port = map["server"]["port"].clone().unwrap().parse().unwrap();
        self.host = map["server"]["host"].clone().unwrap();
    }

    pub fn write_config(&self, config_file: &str){
        let mut config = Ini::new();
        config.set("server", "port", Option::from(self.port.to_string()));
        config.set("server", "host", Option::from(self.host.to_string()));
        config.write(config_file).expect("Failed to write config file");
    }

}