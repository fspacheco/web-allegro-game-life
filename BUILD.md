# Como compilar o projeto com Emscripten

Você precisará do Allegro e do Emscripten

## Emscripten

Baixe de https://github.com/emscripten-core/emsdk

```
cd ~/Downloads/emsdk
./emsdk activate latest
source "~/Downloads/emsdk/emsdk_env.sh"
```

## Allegro

Baixe o código-fonte do Allegro 5 de https://github.com/liballeg/allegro5/releases e compile

```
export BASE_ALLEGRO=(~/Downloads/biblioteca/allegro5-master)

export USE_FLAGS=(    -s USE_FREETYPE=1    -s USE_VORBIS=1    -s USE_OGG=1    -s USE_LIBJPEG=1    -s USE_SDL=2    -s USE_LIBPNG=1    -sFULL_ES2=1    -s ASYNCIFY    -s TOTAL_MEMORY=2147418112    -O3    )
```

Agora você precisa das bibliotecas do Allegro. Você pode usar diretamente do diretório do Allegro ou copiar para dentro do projeto. Aqui vamos copiar
de allegro5-master/build/lib/

```
cd ~/proj/web-allegro-game-life

mkdir lib_LINUX
cp ~/Downloads/biblioteca/allegro5-master/build/lib/ lib_LINUX

ln -s liballegro_acodec.so.5.0.6 liballegro_acodec.so.5.0
ln -s liballegro_audio.so.5.0.6 liballegro_audio.so.5.0
ln -s liballegro_color.so.5.0.6 liballegro_color.so.5.0
ln -s liballegro_dialog.so.5.0.6 liballegro_dialog.so.5.0
ln -s liballegro_font.so.5.0.6 liballegro_font.so.5.0
ln -s liballegro_image.so.5.0.6 liballegro_image.so.5.0
ln -s liballegro_main.so.5.0.6 liballegro_main.so.5.0
ln -s liballegro_memfile.so.5.0.6 liballegro_memfile.so.5.0
ln -s liballegro_physfs.so.5.0.6 liballegro_physfs.so.5.0
ln -s liballegro_primitives.so.5.0.6 liballegro_primitives.so.5.0
ln -s liballegro.so.5.06. liballegro.so.5.0
ln -s liballegro_ttf.so.5.0.6 liballegro_ttf.so.5.0

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~proj/web-allegro-gamelife/lib_LINUX
export LD_LIBRARY_PATH
```

## Compilação

Finalmente compilar. A compilação é bem mais lenta que com gcc/g++

```
emcc main.c -DALLEGRO_STATICLINK $BASE_ALLEGRO/build-emscripten/lib/liballegro_monolith-static.a -I$BASE_ALLEGRO/include/ -I$BASE_ALLEGRO/build-emscripten/include/ -I$BASE_ALLEGRO/addons/primitives/ -I$BASE_ALLEGRO/addons/font/  -I$BASE_ALLEGRO/addons/ttf/ -I$BASE_ALLEGRO/addons/image/ -I$BASE_ALLEGRO/addons/audio/ -I$BASE_ALLEGRO/addons/acodec/ $USE_FLAGS -s ERROR_ON_UNDEFINED_SYMBOLS=0 -o allegro-life.html --preload-file assets --preload-file cells
```

## Teste

Para testar localmente, você precisa de um servidor web. Só abrir diretamente o html não funciona.

```
python3 -m http.server
```

# IMPORTANTE

Quando for desenhar na tela (rotina de *redraw*), use primeiro `al_clear_to_color(al_map_rgb(0,0,0));`, desenhe os elementos e depois chame `al_flip_display();`. Inverter funciona na versão compilada com gcc/g++, mas com emcc fica muito ruim, com um **flicker**.

```
        if(redraw && al_is_event_queue_empty(event_queue)) {
            al_clear_to_color(al_map_rgb(0,0,0));
            al_draw_filled_circle(pos_x, pos_y, BOUNCER_SIZE, al_map_rgb(255,0,255));
            al_flip_display();
            redraw = false;
        }
```
