package controller

import (
	"context"

	. "github.com/onsi/ginkgo/v2"
	. "github.com/onsi/gomega"

	appsv1 "k8s.io/api/apps/v1"
	networkingv1 "k8s.io/api/networking/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/apimachinery/pkg/types"
	"sigs.k8s.io/controller-runtime/pkg/reconcile"

	appsv1alpha1 "github.com/ivan-digital/vecodex/kube-operator/api/v1alpha1"
)

var _ = Describe("VecodexSet Controller", func() {
	const (
		resourceName = "test-resource"
		namespace    = "default"
	)

	var (
		ctx context.Context
	)

	BeforeEach(func() {
		ctx = context.Background()
	})

	Context("Reconciliation logic", func() {
		It("should create necessary deployments and ingress for VecodexSet", func() {
			typeNamespacedName := types.NamespacedName{Name: resourceName, Namespace: namespace}

			By("Creating the VecodexSet resource")
			vecodexSet := &appsv1alpha1.VecodexSet{
				ObjectMeta: metav1.ObjectMeta{
					Name:      resourceName,
					Namespace: namespace,
				},
				Spec: appsv1alpha1.VecodexSetSpec{
					Coordinator: appsv1alpha1.NodeSpec{
						Image:    "nginx:1.21",
						Replicas: int32Ptr(1),
					},
					Writer: appsv1alpha1.NodeSpec{
						Image:    "nginx:1.21",
						Replicas: int32Ptr(2),
					},
					Searcher: appsv1alpha1.NodeSpec{
						Image:    "nginx:1.21",
						Replicas: int32Ptr(3),
					},
					Ingress: appsv1alpha1.IngressSpec{
						Host: "example.com",
					},
				},
			}
			Expect(k8sClient.Create(ctx, vecodexSet)).To(Succeed())

			By("Reconciling the resource")
			controllerReconciler := &VecodexSetReconciler{
				Client: k8sClient,
				Scheme: k8sClient.Scheme(),
			}
			_, err := controllerReconciler.Reconcile(ctx, reconcile.Request{NamespacedName: typeNamespacedName})
			Expect(err).NotTo(HaveOccurred())

			By("Verifying that the coordinator deployment is created")
			deployment := &appsv1.Deployment{}
			Expect(k8sClient.Get(ctx, types.NamespacedName{Name: resourceName + "-coordinator", Namespace: namespace}, deployment)).To(Succeed())
			Expect(deployment.Spec.Replicas).To(Equal(int32Ptr(1)))

			By("Verifying that the writer deployment is created")
			Expect(k8sClient.Get(ctx, types.NamespacedName{Name: resourceName + "-writer", Namespace: namespace}, deployment)).To(Succeed())
			Expect(deployment.Spec.Replicas).To(Equal(int32Ptr(2)))

			By("Verifying that the searcher deployment is created")
			Expect(k8sClient.Get(ctx, types.NamespacedName{Name: resourceName + "-searcher", Namespace: namespace}, deployment)).To(Succeed())
			Expect(deployment.Spec.Replicas).To(Equal(int32Ptr(3)))

			By("Verifying that the ingress is created")
			ingress := &networkingv1.Ingress{}
			Expect(k8sClient.Get(ctx, types.NamespacedName{Name: resourceName + "-ingress", Namespace: namespace}, ingress)).To(Succeed())
			Expect(ingress.Spec.Rules[0].Host).To(Equal("example.com"))
		})
	})
})

func int32Ptr(i int32) *int32 {
	return &i
}
