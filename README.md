FFFS
====

* FFFS - Fernando Ferraz File System :P

 Este file system tem o intuito de servir como um ambiente de aprendizado
 para o desenvolvimento de file systems sob linux VFS. Contribuições
 são bem vindas tendo em vista que este código pode servir para aprendizado
 de mais pessoas no futuro. Este File system segue como referência o
 código publicado no artigo "Creating Linux virtual filesystems" por Jonathan
 Corbet - http://lwn.net/Articles/57369/ (código disponível em
 https://gist.github.com/prashants/3496839)

* Funcionalidades (todas dummy :P):
   /fibonacci:
     - read_file(): calcula próximo fibonnaci para cada operação de leitura
     no arquivo fibonacci.
     - write_file(): verifica se valor passado é um numero fibonacci, caso
     verdadeiro o valor é armazenado.

