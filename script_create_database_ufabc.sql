CREATE database db_estacionamento_ufabc;

use db_estacionamento_ufabc;

CREATE TABLE tb_categoria (
cod_categoria TINYINT Unsigned not null,
descricao varchar(50) not null,
PRIMARY KEY (cod_categoria)
);

INSERT INTO tb_categoria ( cod_categoria, descricao) VALUES ( 0, 'ALUNO'), ( 1, 'PROFESSOR'), ( 2, 'TECNICO'), ( 3, 'VISITANTE');

CREATE TABLE tb_usuario (
cod_usuario varchar(20) not null,
nome varchar(255),
cod_categoria TINYINT Unsigned not null,
PRIMARY KEY (cod_usuario),
FOREIGN KEY (cod_categoria) REFERENCES tb_categoria(cod_categoria));

INSERT INTO tb_usuario ( cod_usuario, nome, cod_categoria) VALUES ( '21017114', 'Nicolas Rafael Thales Moraes', 0), ( '21177133', 'Isabelle Beatriz Sales', 0), ( '13211721638', 'Débora Carla Olivia Barros', 0), ( '11201721630', 'Maitê Débora Teixeira', 1), ( '11201810614', 'Tânia Sabrina Gonçalves', 2), ( '11201810615', 'Agatha Stefany Sales', 3);

CREATE TABLE tb_rfid (
cod_usuario varchar(20) not null,
cod_rfdi varchar(20) not null,
PRIMARY KEY (cod_rfdi),
FOREIGN KEY (cod_usuario) REFERENCES tb_usuario(cod_usuario));

INSERT INTO tb_rfid ( cod_rfdi, cod_usuario) VALUES ( '41 85 AF A9', '21017114'),( '42 85 AF A9', '21177133'),( '43 85 AF A9', '13211721638'),( '44 85 AF A9', '11201721630'),( '45 85 AF A9', '11201810614'),( '46 85 AF A9', '11201810615');

CREATE TABLE tb_controle_acesso (
id_controle_acesso INT unsigned auto_increment,
cod_status TINYINT Unsigned not null,
data_entrada datetime not null,
data_saida datetime not null,
cod_usuario varchar(20) not null,
PRIMARY KEY (id_controle_acesso),
FOREIGN KEY (cod_usuario) REFERENCES tb_usuario(cod_usuario));


DELIMITER //
create PROCEDURE sp_users_list(IN pcod_usuario VARCHAR(20))
USE db_estacionamento_ufabc;
BEGIN
	IF pcod_usuario = -1 THEN
	
		SELECT * FROM tb_usuario;
		
	ELSE
	
		SELECT * FROM tb_usuario WHERE cod_usuario = pcod_usuario;
	
	END IF;
END //
DELIMITER ;




