FFFS - Fernando Ferraz File System :P
====

* Introdução

 Este file system tem o intuito de servir como um ambiente de aprendizado
 para o desenvolvimento de file systems sob linux VFS. Contribuições
 são bem vindas tendo em vista que este código pode servir para aprendizado
 de mais pessoas no futuro. Este File system segue como referência o
 código publicado no artigo "Creating Linux virtual filesystems" por Jonathan
 Corbet - http://lwn.net/Articles/57369/ (código disponível em
 https://gist.github.com/prashants/3496839).

* Funcionalidades (todas dummy :P)
  
  - arquivo /fibonacci:
     - read_file(): exibe próximo fibonacci para cada operação de leitura
                  no arquivo fibonacci.
     - write_file(): verifica se valor é um numero fibonacci, caso
                   verdadeiro o valor é armazenado.


* Instruções
  
  - criar dispositivo de loopback para montar o file system:
      - dd if=/dev/zero of=/home/mo806i/rep bs=1k count=4

  - criar diretório para montar file system:
      - mkdir -p /home/mo806ii/mnt/fffs/

  - montar o file system:
      - sudo mount -t fffs -o loop /home/mo806i/rep  /home/mo806i/mnt/fffs/

  - lendo arquivo fibonacci:
      - cat fibonacci

  - escrevendo no arquivo fibonacci:
      - echo 13 > fibonacci


