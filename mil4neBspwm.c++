#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <filesystem>
#include <string>
#include <sys/stat.h>
#include <fstream>
#include <sys/types.h>
#include <vector>
#include <sys/wait.h>

using namespace std;
namespace fs = std::filesystem;

string input_username;
string directorio_instalacion;
std::string p10k_repo = "https://github.com/romkatv/powerlevel10k.git";

int check_user_permissions() {
    if (geteuid() != 0) {
        cout << "\033[31m[*]\033[0m Ejecuta este programa con sudo." << endl;
        exit(0);
        return 1;
    }
    cout << "El usuario tiene permisos de sudo." << endl;
    return 0;
}

int get_user() {
    cout << "\033[33m[*]\033[0m username: ";
    cin >> input_username;
    return 0;
}

int directorio() {
    filesystem::path directorio_instalacion = filesystem::current_path();
    cout << "El directorio de instalación actual es: " << directorio_instalacion << endl;
    return 0;
}

int update() {
    while (true) {
        string respuesta_update;
        cout << "\033[33m[*]\033[0m Update al Sistema? (SI/NO) -->";
        cin >> respuesta_update;

        for (char &c : respuesta_update) {
            c = tolower(c);
        }

        if (respuesta_update == "si" || respuesta_update == "s") {
            cout << "\033[32m[*]\033[0m Ejecutando 'apt update' ...\n";
            system("apt update -y");
            break;
        } else if (respuesta_update == "no" || respuesta_update == "n") {
            cout << "\033[31m[*]\033[0m Operación 'apt update' cancelada.\n";
            break;
        } else {
            cout << "\033[31m[*]\033[0m Respuesta no válida. responde 'SI' o 'NO'.\n";
        }
    }

    return 0;
}

int upgrade() {
    while (true) {
        string respuesta_upgrade;
        cout << "\033[33m[*]\033[0m Upgrade al Sistema? (SI/NO) -->  ";
        cin >> respuesta_upgrade;

        for (char &c : respuesta_upgrade) {
            c = tolower(c);
        }

        if (respuesta_upgrade == "si" || respuesta_upgrade == "s") {
            cout << "\033[32m[*]\033[0m Ejecutando 'apt upgrade' ...\n";
            system("apt upgrade -y");
            break;
        } else if (respuesta_upgrade == "no" || respuesta_upgrade == "n") {
            cout << "\033[31m[*]\033[0m Operación 'apt update' cancelada.\n";
            break;
        } else {
            cout << "\033[31m[*]\033[0m Respuesta no válida. responde 'SI' o 'NO'.\n";
        }
    }

    return 0;
}



void eliminarArchivos(const std::string& ruta) {
    // Eliminar archivos individuales
    fs::remove(ruta + "/.zshrc");
    fs::remove(ruta + "/.p10k.zsh");

    // Eliminar archivos en .config
    fs::remove_all(ruta + "/.config/polybar");
    fs::remove_all(ruta + "/.config/bspwm");
    fs::remove_all(ruta + "/.config/sxhkd");
    fs::remove_all(ruta + "/.config/kitty"); // Aquí se elimina recursivamente

    // Eliminar archivos individuales de /root
    fs::remove("/root/.zshrc");
    fs::remove("/root/.p10k.zsh");
    fs::remove_all("/root/.config/kitty"); // Aquí se elimina recursivamente

    // Eliminar archivos en powerlevel10k
    fs::remove_all(ruta + "/powerlevel10k");
    fs::remove_all("/root/powerlevel10k");
}


int dependencias() {
    std::cout << "\033[32m[*]\033[0m Instalando Dependencias necesarias ...\n";

    // Crear un nuevo proceso hijo para ejecutar apt install
    pid_t pid = fork();

    if (pid == -1) {
        std::cerr << "Error al crear el proceso hijo." << std::endl;
        return -1;
    } else if (pid == 0) { // Código para el proceso hijo
        // Lista de argumentos para apt install
        char *args[] = {
            (char*)"apt",
            (char*)"install",
            (char*)"feh",
            (char*)"xclip",
            (char*)"bspwm",
            (char*)"sxhkd",
            (char*)"wmname",
            (char*)"bat",
            (char*)"polybar",
            (char*)"lsd",
            (char*)"fzf",
            (char*)"flameshot",
            (char*)"picom",
            (char*)"rofi",
            (char*)"kitty",
            (char*)"zsh",
            (char*)"-y",
            nullptr
        };

        // Reemplazar la imagen del proceso hijo por apt install
        execvp(args[0], args);

        // Si execvp() devuelve, ha habido un error
        std::cerr << "Error al ejecutar apt install." << std::endl;
        exit(EXIT_FAILURE);
    } else { // Código para el proceso padre
        // Esperar a que el proceso hijo termine
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            std::cout << "\033[32m[*]\033[0m Dependencias Instaladas...\n";
        } else {
            std::cout << "\033[31m[*]\033[0m Error al instalar Dependencias\n";
        }
    }

    return 0;
}


int nuevas_carpetas(const std::string& input_username) {
    try {
        // Rutas de las carpetas a crear
        std::string root_config_dir = "/root/.config";
        std::string user_config_dir = "/home/" + input_username + "/.config";

        // Crear la carpeta .config en el directorio raíz
        if (!fs::create_directory(root_config_dir)) {
            throw std::runtime_error("No se pudo crear la carpeta .config en el directorio raíz.");
        }

        // Crear la carpeta .config en el directorio del usuario
        if (!fs::create_directory(user_config_dir)) {
            throw std::runtime_error("No se pudo crear la carpeta .config en el directorio del usuario.");
        }

        std::cout << "\033[32m[*]\033[0m Nuevas carpetas .config Creadas.\n";
    } catch (const std::exception& e) {
        std::cerr << "\033[31m[*]\033[0m Error al crear las nuevas carpetas .config: " << e.what() << "\n";
    }

    return 0;
}

int fuentes() {
    try {
        std::string fonts_src_dir = "fonts";
        std::string fonts_dest_dir = "/usr/local/share/fonts";

        if (!fs::exists(fonts_dest_dir)) {
            if (!fs::create_directories(fonts_dest_dir)) {
                throw std::runtime_error("No se pudo crear el directorio de destino para las fuentes.");
            }
        }

        fs::copy(fonts_src_dir, fonts_dest_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);

        std::cout << "\033[32m[*]\033[0m Fuentes Instaladas.\n";
    } catch (const std::exception& e) {
        std::cerr << "\033[31m[*]\033[0m Error al instalar fuentes: " << e.what() << "\n";
    }

    return 0;
}

void kitty_conf(const std::string& username)  {
    std::string ccommand = "mkdir /home/" + username + "/.config/kitty";
    system(ccommand.c_str());
    system("mkdir /root/.config/kitty");
    fs::copy("kitty", "/home/" + username + "/.config/kitty", fs::copy_options::recursive);
    fs::copy("kitty", "/root/.config", fs::copy_options::recursive);
    std::cout << "\033[32m[*]\033[0m Config de Kitty instalada.\n";
}

void polybar(const std::string& input_username, const std::string& directorio_instalacion) {
    std::cout << "\033[32m[*]\033[0m Configurando polybar ...\n";
    
    system(("mkdir /home/" + input_username + "/.config/polybar").c_str());
   
    fs::copy("polybar", "/home/" + input_username + "/.config/polybar", fs::copy_options::recursive);

    
    std::string scripts_path = "/home/" + input_username + "/.config/polybar/scripts";
    for (const auto& entry : fs::directory_iterator(scripts_path)) {
        chmod(entry.path().c_str(), S_IRWXU);
    }

   
    fs::create_directory("/home/" + input_username + "/.config/bin");
    std::ofstream target_file("/home/" + input_username + "/.config/bin/target");
    target_file.close();

    chdir(directorio_instalacion.c_str());
}

int sxhkd_conf(const std::string& username) {
    std::string command = "mkdir /home/" + username + "/.config/sxhkd";
    system(command.c_str());
    fs::copy("sxhkd", "/home/" + username + "/.config/sxhkd", fs::copy_options::recursive);
    std::cout << "\033[32m[*]\033[0m Config de Sxhkd instalada.\n";
    return 0;
}

int su_plugin() {
    std::string source_dir = "zsh-sudo"; 
    std::string destination_dir = "/usr/share"; 

    try {
        fs::copy(source_dir, destination_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        std::cout << "\033[32m[*]\033[0m Sudo Plugin Instaladas.\n";
    } catch (const std::exception& e) {
        std::cerr << "\033[31m[*]\033[0m Error al instalar Sudo Plugin: " << e.what() << "\n";
    }

    return 0;
}

void bspwm_conf(const std::string& input_username, const std::string& directorio_instalacion) {
    std::string cccommand = "mkdir /home/" + input_username + "/.config/bspwm";
    system(cccommand.c_str());
    fs::copy("bspwm", "/home/" + input_username + "/.config/bspwm", fs::copy_options::recursive);

    std::string bspwmrc_path = "/home/" + input_username + "/.config/bspwm/bspwmrc";
    chmod(bspwmrc_path.c_str(), S_IRWXU);

    std::string scripts_path = "/home/" + input_username + "/.config/bspwm/scripts";
    for (const auto& entry : fs::directory_iterator(scripts_path)) {
        chmod(entry.path().c_str(), S_IRWXU);
    }

    std::cout << "\033[32m[*]\033[0m Config de Bspwm instalada.\n";
}

int rofi_conf() {
    std::string command = "mkdir /home/" + input_username + "/.config/rofi";
    system(command.c_str());
    system(("mkdir /home/" + input_username + "/.config/rofi/launcher").c_str());
    system(("mkdir /home/" + input_username + "/.config/rofi/powermenu").c_str());

    std::string source_dir = "rofi"; 
    std::string user_config_dir = "/home/" + input_username + "/.config";

    try {
        fs::copy(source_dir, user_config_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);

        fs::permissions(user_config_dir + "/rofi/launcher/launcher.sh", fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec);
        fs::permissions(user_config_dir + "/rofi/powermenu/powermenu.sh", fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec);

        std::cout << "\033[32m[*]\033[0m Configuración de Rofi instalada correctamente.\n";
    } catch (const std::exception& e) {
        std::cerr << "\033[31m[*]\033[0m Error al configurar Rofi: " << e.what() << "\n";
    }

    return 0;
}

int p10k_conf() {
    std::string user_home_dir = "/home/" + input_username;

    try {
        std::string clone_command = "git clone --depth=1 " + p10k_repo + " " + user_home_dir + "/powerlevel10k";
        if (system(clone_command.c_str()) != 0) {
            throw std::runtime_error("Error al clonar el repositorio de powerlevel10k.");
        }

        std::ofstream zshrc_file(user_home_dir + "/.zshrc", std::ios_base::app);
        if (!zshrc_file.is_open()) {
            throw std::runtime_error("No se pudo abrir el archivo .zshrc.");
        }
        zshrc_file << "\nsource ~/powerlevel10k/powerlevel10k.zsh-theme\n";
        zshrc_file.close();

        fs::rename("zshrc", user_home_dir + "/.zshrc");
        fs::rename("p10k.zsh", user_home_dir + "/.p10k.zsh");

        std::cout << "\033[32m[*]\033[0m Configuración de powerlevel10k instalada correctamente.\n";
    } catch (const std::exception& e) {
        std::cerr << "\033[31m[*]\033[0m Error en la configuración de powerlevel10k: " << e.what() << "\n";
    }

    return 0;
}

int p10k_conf_root() {
    std::string root_home_dir = "/root";

    try {
        std::string clone_command = "git clone --depth=1 " + p10k_repo + " " + root_home_dir + "/powerlevel10k";
        if (system(clone_command.c_str()) != 0) {
            throw std::runtime_error("Error al clonar el repositorio de powerlevel10k en el directorio /root.");
        }

        std::ofstream zshrc_file(root_home_dir + "/.zshrc", std::ios_base::app);
        if (!zshrc_file.is_open()) {
            throw std::runtime_error("No se pudo abrir el archivo .zshrc de root.");
        }
        zshrc_file << "\nsource ~/powerlevel10k/powerlevel10k.zsh-theme\n";
        zshrc_file.close();

        fs::copy_file(".p10k.zsh", root_home_dir + "/.p10k.zsh", fs::copy_options::overwrite_existing);
        fs::copy_file(".zshrc", root_home_dir + "/.zshrc", fs::copy_options::overwrite_existing);

        std::cout << "\033[32m[*]\033[0m Configuración de powerlevel10k para root instalada correctamente.\n";
    } catch (const std::exception& e) {
        std::cerr << "\033[31m[*]\033[0m Error en la configuración de powerlevel10k para root: " << e.what() << "\n";
    }

    return 0;
}

int user_replace() {
    system(("sed -i 's/byt3/" + input_username + "/g' /home/" + input_username + "/.config/polybar/*").c_str());
    system(("sed -i 's/byt3/" + input_username + "/g' /home/" + input_username + "/.config/polybar/scripts/*").c_str());

    system(("sed -i 's/byt3/" + input_username + "/g' /home/" + input_username + "/.config/bspwm/*").c_str());
    system(("sed -i 's/byt3/" + input_username + "/g' /home/" + input_username + "/.config/bspwm/scripts/*").c_str());

    system(("sed -i 's/byt3/" + input_username + "/g' /home/" + input_username + "/.config/sxhkd/*").c_str());

    system(("sed -i 's/byt3/" + input_username + "/g' /home/" + input_username + "/.p10k.zsh").c_str());
    system(("sed -i 's/byt3/" + input_username + "/g' /home/" + input_username + "/.zshrc").c_str());
    return 0;
}

void Wallpaper(const std::string& rutaOrigen, const std::string& rutaDestino) {
    std::string command = "mkdir /home/" + input_username + "/Wallpapers";
    system(command.c_str());
    fs::copy(rutaOrigen, rutaDestino, fs::copy_options::recursive);
}


int main() {
    // Verifica ejecucion con sudo   
    check_user_permissions();

    // Agarrra el username
    get_user();

    // pwd actual
    directorio();

    // Update al Sistema
    update();

    // Upgrade al Sistema
    upgrade();

    // Borrando configuraciones antiguas
    std::cout << "\033[32m[*]\033[0m Eliminando Config Antiguas ...\n";
    eliminarArchivos("/home/" + input_username);

    // Instalando Dependecias para entorno
    dependencias();

    // Creando Nuevas carpetas .config
    nuevas_carpetas(input_username);

    // Instalando Fuentes
    fuentes();

    // Instalando config Kitty
    kitty_conf(input_username);
 
    // Instalando config Sxhkd
    sxhkd_conf(input_username);

    // Instalando sudo Plugin
    su_plugin();

    // Instalando config Bspwm
    bspwm_conf(input_username, directorio_instalacion);

    // Instalando config Rofi
    rofi_conf();

    polybar(input_username, directorio_instalacion);

    // Instalando config p10k
    p10k_conf();

    // Instalando config p10k root
    p10k_conf_root();

    user_replace();

    // configurar Wallpaper
    std::string rutaOrigen = "Wallpapers";
    std::string rutaDestino = "/home/" + input_username + "/Wallpapers";
    std::cout << "\033[32m[*]\033[0m Configurando wallpapers ...\n";
    Wallpaper(rutaOrigen, rutaDestino);

    // Crear un enlace simbólico entre la .zshrc del usuario elegido y la .zshrc de root
    system(("ln -s -f /home/" + input_username + "/.zshrc /root/.zshrc").c_str());
    std::cout << "\033[32m[*]\033[0m Creando link simbólico en la .zshrc ...\n";

    // Crear un enlace simbólico entre el .p10k.zsh del usuario elegido y el de root
    system(("ln -s -f /home/" + input_username + "/.p10k.zsh /root/.p10k.zsh").c_str());
    std::cout << "\033[32m[*]\033[0m Creando link simbólico en el archivo .p10k.zsh ...\n";

    // Asignar el propietario correcto a los archivos de configuración del usuario
    system(("chown -R " + input_username + ":" + input_username + " /home/" + input_username).c_str());

    return 0;
}


