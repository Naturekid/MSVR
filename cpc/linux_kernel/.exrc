if &cp | set nocp | endif
let s:cpo_save=&cpo
set cpo&vim
map! <S-Insert> <MiddleMouse>
map =ex :Ex
nmap gx <Plug>NetrwBrowseX
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#NetrwBrowseX(expand("<cWORD>"),0)
map <F7> A#endif
map <F6> ^ik^i#if 0
map <F5> ^xx$xx
map <F4> ^i/*$a*/
map <F3> :set nohlsearch
map <F2> :set hlsearch
map <S-Insert> <MiddleMouse>
map! =#i #include <
map! =pr printf(");hi
map! =v void
map! =c char
map! =l long
map! =f float
map! =d double
map! =i int
map! =s short
map! =}  {}kA
map! ={ {}kA
imap =ex :Ex
let &cpo=s:cpo_save
unlet s:cpo_save
set background=dark
set backspace=indent,eol,start
set fileencodings=ucs-bom,utf-8,default,latin1
set guifont=Bitstream\ Vera\ Sans\ Mono\ 12
set guioptions=aegimrLt
set noguipty
set helplang=en
set history=50
set ignorecase
set incsearch
set nomodeline
set mouse=a
set printoptions=paper:letter
set ruler
set runtimepath=~/.vim,/var/lib/vim/addons,/usr/share/vim/vimfiles,/usr/share/vim/vim72,/usr/share/vim/vimfiles/after,/var/lib/vim/addons/after,~/.vim/after
set shiftwidth=4
set showcmd
set showmatch
set smartcase
set suffixes=.bak,~,.swp,.o,.info,.aux,.log,.dvi,.bbl,.blg,.brf,.cb,.ind,.idx,.ilg,.inx,.out,.toc
set tabstop=4
set termencoding=utf-8
set window=50
" vim: set ft=vim :
