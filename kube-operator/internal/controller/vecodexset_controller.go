package controller

import (
	"context"

	"github.com/go-logr/logr"
	appsv1alpha1 "github.com/ivan-digital/vecodex/kube-operator/api/v1alpha1"
	appsv1 "k8s.io/api/apps/v1"
	corev1 "k8s.io/api/core/v1"
	networkingv1 "k8s.io/api/networking/v1"
	"k8s.io/apimachinery/pkg/api/errors"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/apimachinery/pkg/runtime"
	ctrl "sigs.k8s.io/controller-runtime"
	"sigs.k8s.io/controller-runtime/pkg/client"
	"sigs.k8s.io/controller-runtime/pkg/controller/controllerutil"
	"sigs.k8s.io/controller-runtime/pkg/log"
)

type VecodexSetReconciler struct {
	client.Client
	Scheme *runtime.Scheme
}

// Reconcile implements the reconciliation logic for VecodexSet.
func (r *VecodexSetReconciler) Reconcile(ctx context.Context, req ctrl.Request) (ctrl.Result, error) {
	logger := log.FromContext(ctx)

	// Fetch the VecodexSet instance
	vecodex := &appsv1alpha1.VecodexSet{}
	err := r.Get(ctx, req.NamespacedName, vecodex)
	if err != nil {
		if errors.IsNotFound(err) {
			// Resource not found, nothing to do
			logger.Info("VecodexSet resource not found. Ignoring since it must be deleted.")
			return ctrl.Result{}, nil
		}
		logger.Error(err, "Failed to fetch VecodexSet resource")
		return ctrl.Result{}, err
	}

	// Reconcile deployments for coordinator, writer, and searcher
	err = r.reconcileDeployment(ctx, logger, vecodex, "coordinator", vecodex.Spec.Coordinator)
	if err != nil {
		return ctrl.Result{}, err
	}
	err = r.reconcileDeployment(ctx, logger, vecodex, "writer", vecodex.Spec.Writer)
	if err != nil {
		return ctrl.Result{}, err
	}
	err = r.reconcileDeployment(ctx, logger, vecodex, "searcher", vecodex.Spec.Searcher)
	if err != nil {
		return ctrl.Result{}, err
	}

	// Reconcile Ingress
	err = r.reconcileIngress(ctx, logger, vecodex)
	if err != nil {
		return ctrl.Result{}, err
	}

	logger.Info("Successfully reconciled VecodexSet")
	return ctrl.Result{}, nil
}

func (r *VecodexSetReconciler) reconcileDeployment(ctx context.Context, logger logr.Logger, vecodex *appsv1alpha1.VecodexSet, name string, spec appsv1alpha1.NodeSpec) error {
	deployment := &appsv1.Deployment{
		ObjectMeta: metav1.ObjectMeta{
			Name:      vecodex.Name + "-" + name,
			Namespace: vecodex.Namespace,
		},
	}
	_, err := controllerutil.CreateOrUpdate(ctx, r.Client, deployment, func() error {
		replicas := spec.Replicas
		deployment.Spec = appsv1.DeploymentSpec{
			Replicas: replicas,
			Selector: &metav1.LabelSelector{
				MatchLabels: map[string]string{"app": vecodex.Name + "-" + name},
			},
			Template: corev1.PodTemplateSpec{
				ObjectMeta: metav1.ObjectMeta{
					Labels: map[string]string{"app": vecodex.Name + "-" + name},
				},
				Spec: corev1.PodSpec{
					Containers: []corev1.Container{
						{
							Name:  name,
							Image: spec.Image,
						},
					},
				},
			},
		}
		return controllerutil.SetControllerReference(vecodex, deployment, r.Scheme)
	})
	if err != nil {
		logger.Error(err, "Failed to reconcile Deployment", "name", name)
		return err
	}
	logger.Info("Reconciled Deployment", "name", name)
	return nil
}

func (r *VecodexSetReconciler) reconcileIngress(ctx context.Context, logger logr.Logger, vecodex *appsv1alpha1.VecodexSet) error {
	ingress := &networkingv1.Ingress{
		ObjectMeta: metav1.ObjectMeta{
			Name:      vecodex.Name + "-ingress",
			Namespace: vecodex.Namespace,
		},
	}
	_, err := controllerutil.CreateOrUpdate(ctx, r.Client, ingress, func() error {
		pathType := networkingv1.PathTypePrefix
		ingress.Spec = networkingv1.IngressSpec{
			Rules: []networkingv1.IngressRule{
				{
					Host: vecodex.Spec.Ingress.Host,
					IngressRuleValue: networkingv1.IngressRuleValue{
						HTTP: &networkingv1.HTTPIngressRuleValue{
							Paths: []networkingv1.HTTPIngressPath{
								{
									Path:     "/",
									PathType: &pathType,
									Backend: networkingv1.IngressBackend{
										Service: &networkingv1.IngressServiceBackend{
											Name: vecodex.Name + "-coordinator",
											Port: networkingv1.ServiceBackendPort{
												Number: 80,
											},
										},
									},
								},
							},
						},
					},
				},
			},
		}
		return controllerutil.SetControllerReference(vecodex, ingress, r.Scheme)
	})
	if err != nil {
		logger.Error(err, "Failed to reconcile Ingress")
		return err
	}
	logger.Info("Reconciled Ingress")
	return nil
}

// SetupWithManager sets up the controller with the Manager.
func (r *VecodexSetReconciler) SetupWithManager(mgr ctrl.Manager) error {
	return ctrl.NewControllerManagedBy(mgr).
		For(&appsv1alpha1.VecodexSet{}).
		Complete(r)
}
