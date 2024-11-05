# Projeto_AED

**Como instalar a lib:**

**Para Linux:**

```
sudo apt-get update
sudo apt-get install libsdl2-dev
```

**Para MacOS:**

```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew update
brew install sdl2
```


**Para Windons:**

* Acesse a página de download da SDL2: [https://www.libsdl.org/download-2.0.php.]()


* Extraia o conteúdo do arquivo ZIP baixado para um local de sua escolha.
* Inclua os arquivos .h do SDL2 no diretório de cabeçalhos do seu projeto.

* Adicione os arquivos .lib na pasta de bibliotecas do seu projeto.
* Configure seu compilador para usar o SDL2 (isso pode variar dependendo do IDE que você está usando).

**Como buildar o jogo:**

```
gcc main.c -o main -lSDL2
```

**Como executar:**

```
./main
```

