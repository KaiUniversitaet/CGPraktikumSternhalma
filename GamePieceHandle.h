#pragma once
#include "Model.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
double wurzel2 = 1.414213562f;
double aspectLength = 1 / 17.0;
double aspectWidth = 1 / 25.0;
class GamePieceHandle {

	Model* model_;
	class Shader* shader_;
	GLchar* path_;
	glm::vec2 fieldCenter;


public:


	enum Color {
		RED,
		BLUE,
		YELLOW,
		GREEN,
		WHITE
	};

	struct Piece {
		glm::vec3 position;
		glm::vec3 color;
		GLfloat boden;
	};

	std::vector <Piece*> pieces_;

	enum DIRECTION{
		EAST,
		SEAST,
		SWEST,
		WEST,
		NWEST,
		NEAST
	};

	bool animate(int index, DIRECTION dir, GLfloat deltaTime, GLfloat durationJump, GLfloat distance) {
		static GLfloat zurGelegterWeg = 0.0f;
		GLfloat step = deltaTime*distance / durationJump;
		if (index < pieces_.size() && zurGelegterWeg < distance) {
			glm::vec3 go{distance,0.0f,distance};
			GLfloat xdiag= go.x *glm::cos(glm::radians(60.0f));
			GLfloat zdiag= go.z *glm::sin(glm::radians(60.0));
			switch (dir) {
			case NEAST: go.z =-zdiag;
				go.x = xdiag;
				break;
			case SEAST:
				go.x = xdiag; go.z = zdiag;
				break;
			case WEST:
				go.x = -distance;
				go.z = 0;
				break;
			case EAST:
				go.x = distance; go.z = 0.0f;
				break;
			case NWEST:
				go.x = -xdiag; go.z = -zdiag;
				break;
			case SWEST:
				go.x = -xdiag;
				go.z = zdiag;
				break;
			default:
				break;
			}
			go *= step;
			zurGelegterWeg += sqrt(go.x*go.x + go.z*go.z);
			GLfloat anteilAnGesamtStrecke = zurGelegterWeg / distance;
			pieces_[index]->position += go;
			pieces_[index]->position.y = std::max( double(pieces_[index]->boden + (glm::sin(  anteilAnGesamtStrecke * M_PI )*0.3)),double (pieces_[index]->boden) );
			return true;
		}
		else if (zurGelegterWeg >= distance) {
			zurGelegterWeg = 0.0f;
			return false;
		}

	}



	GamePieceHandle(GLchar* path, GLchar* vertexPath, GLchar* fragmentPath, double boardlength, double boardwidth) {

		double bl = boardlength;
		double bw = boardwidth;

		setModel(path);
		setShader(vertexPath, fragmentPath);
	}

	void addPiece(glm::vec3 position, Color c) {
		Piece* p = new Piece;
		p->boden = position.y;
		switch (c) {
		case RED:
			p->color = glm::vec3(1.0f, 0.0f, 0.0f); break;
		case BLUE:
			p->color = glm::vec3(0.0f, 1.0f, 0.0f); break;
		case GREEN:
			p->color = glm::vec3(0.0f, 0.0f, 1.0f); break;
		default:
			p->color = glm::vec3(1.0f); break;
		}

		p->position = position;
		pieces_.push_back(p);;
	}




	void setModel(GLchar* path) {
		path_ = path;
		model_ = new Model(path);
	}

	void setShader(const GLchar* vertexPath, const GLchar* fragmentPath) {
		shader_ = new Shader(vertexPath, fragmentPath);
	}


	void drawPieces(glm::mat4 view, glm::mat4 projection, glm::vec3 lightPos, Camera &camera) {
		// draw the pawns

		shader_->Use();
		// Transformation matrices
		glUniformMatrix4fv(glGetUniformLocation(shader_->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader_->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		for (auto p : pieces_) {

			// Draw the loaded model
			glm::mat4 model1;

			GLint objectColorLoc = glGetUniformLocation(shader_->Program, "objectColor");
			GLint lightColorLoc = glGetUniformLocation(shader_->Program, "lightColor");
			GLint lightPosLoc = glGetUniformLocation(shader_->Program, "lightPos");
			GLint viewPosLoc = glGetUniformLocation(shader_->Program, "viewPos");
			glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
			glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
			glUniform3f(objectColorLoc, p->color.r, p->color.b, p->color.g);
			glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f); // Also set light's color (white)

			model1 = glm::translate(model1, p->position); // Translate it down a bit so it's at the center of the scene
			//model1 = glm::rotate(model1, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model1 = glm::scale(model1, glm::vec3(0.005f, 0.005f, 0.005f));

			glUniformMatrix4fv(glGetUniformLocation(shader_->Program, "model"), 1, GL_FALSE, glm::value_ptr(model1));
			model_->draw(*shader_);
		}




	}





};