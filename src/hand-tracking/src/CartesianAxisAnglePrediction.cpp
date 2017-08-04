#include <utility>

#include "CartesianAxisAnglePrediction.h"

using namespace bfl;
using namespace Eigen;


CartesianAxisAnglePrediction::CartesianAxisAnglePrediction(std::unique_ptr<StateModel> transition_model) noexcept :
    state_model_(std::move(transition_model)) { }


CartesianAxisAnglePrediction::~CartesianAxisAnglePrediction() noexcept { }


CartesianAxisAnglePrediction::CartesianAxisAnglePrediction(CartesianAxisAnglePrediction&& pf_prediction) noexcept :
    state_model_(std::move(pf_prediction.state_model_)) { };


CartesianAxisAnglePrediction& CartesianAxisAnglePrediction::operator=(CartesianAxisAnglePrediction&& pf_prediction) noexcept
{
    state_model_ = std::move(pf_prediction.state_model_);

    return *this;
}


void CartesianAxisAnglePrediction::motion(const Ref<const VectorXf>& cur_state, Ref<VectorXf> prop_state)
{
    state_model_->propagate(cur_state, prop_state);
}


void CartesianAxisAnglePrediction::motionDisturbance(Ref<VectorXf> sample)
{
    state_model_->noiseSample(sample);
}


void CartesianAxisAnglePrediction::predict(const Ref<const VectorXf>& prev_state, Ref<VectorXf> pred_state)
{
    motion(prev_state, pred_state);

    VectorXf sample(VectorXf::Zero(7, 1));
    motionDisturbance(sample);

    VectorXf rotated_vec(VectorXf::Zero(3, 1));
    addAxisangleDisturbance(pred_state.tail<3>(), sample.middleRows<4>(3), rotated_vec);

    pred_state.head<3>() += sample.head<3>();
    pred_state.tail<3>() =  rotated_vec;
}

bool CartesianAxisAnglePrediction::setStateModelProperty(const std::string& property)
{
    return state_model_->setProperty(property);
}


void CartesianAxisAnglePrediction::addAxisangleDisturbance(const Ref<const Vector3f>& current_vec, const Ref<const Vector3f>& disturbance_vec,
                                                           Ref<Vector3f> rotated_vec)
{
    float disturbance_angle = disturbance_vec(3);
    float ang               = current_vec.tail<3>().norm() + disturbance_angle;

    if      (ang >  2.0 * M_PI) ang -= 2.0 * M_PI;
    else if (ang <=        0.0) ang += 2.0 * M_PI;

    /* Find the rotation axis 'u' and rotation angle 'rot' [1] */
    Vector3f def_dir(0.0, 0.0, 1.0);
    Vector3f u = def_dir.cross(current_vec.normalized()).normalized();
    float rot  = static_cast<float>(acos(current_vec.normalized().dot(def_dir)));

    /* Convert rotation axis and angle to 3x3 rotation matrix [2] */
    /* [2] https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle */
    Matrix3f cross_matrix;
    cross_matrix <<     0,  -u(2),   u(1),
                     u(2),      0,  -u(0),
                    -u(1),   u(0),      0;
    Matrix3f R = cos(rot) * Matrix3f::Identity() + sin(rot) * cross_matrix + (1 - cos(rot)) * (u * u.transpose());

    rotated_vec = ang * (R * disturbance_vec).normalized();
}
