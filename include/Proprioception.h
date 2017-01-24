#ifndef PROPRIOCEPTION_H
#define PROPRIOCEPTION_H

#include <exception>
#include <functional>
#include <random>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iCub/iKin/iKinFwd.h>
#include <opencv2/core/core.hpp>
#include <yarp/os/ConstString.h>
#include <yarp/sig/Vector.h>

#include <BayesFiltersLib/VisualObservationModel.h>
#include <SuperImpose/SICAD.h>



class Proprioception : public bfl::VisualObservationModel {
public:
    /* Proprioception complete constructor */
    Proprioception(GLFWwindow* window);

    /* Default constructor */
    Proprioception() = delete;

    /* Destructor */
    ~Proprioception() noexcept override;

    /* Copy constructor */
    Proprioception(const Proprioception& proprio);

    /* Move constructor */
    Proprioception(Proprioception&& proprio) noexcept;

    /* Copy assignment operator */
    Proprioception& operator=(const Proprioception& proprio);

    /* Move assignment operator */
    Proprioception& operator=(Proprioception&& proprio) noexcept;

    void observe(const Eigen::Ref<const Eigen::VectorXf>& cur_state, cv::OutputArray observation) override;

    void setCamXO(double* cam_x, double* cam_o);

    void setImgBackEdge(const cv::Mat& img_back_edge);

    void setArmJoints(const yarp::sig::Vector & q);

    void superimpose(const SuperImpose::ObjPoseMap& obj2pos_map, cv::Mat& img);

protected:
    double                   cam_x_[3];
    double                   cam_o_[4];
    GLFWwindow             * window_;
    iCub::iKin::iCubFinger * icub_kin_finger_[3];
    SICAD                  * si_cad_;
    SuperImpose::ObjFileMap  cad_hand_;

    cv::Mat                  img_back_edge_;

    bool FileFound(const yarp::os::ConstString& file);
};

#endif /* PROPRIOCEPTION_H */
