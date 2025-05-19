/*
Copyright 2024.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

package v1alpha1

import (
	corev1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
)

// VecodexSetSpec defines the desired state of VecodexSet
type VecodexSetSpec struct {
	Coordinator NodeSpec    `json:"coordinator"`
	Writer      NodeSpec    `json:"writer"`
	Searcher    NodeSpec    `json:"searcher"`
	Ingress     IngressSpec `json:"ingress"`
}

type NodeSpec struct {
	Image    string   `json:"image"`
	Replicas *int32   `json:"replicas"`
	Command  []string `json:"command,omitempty"` // Added command field
	// +kubebuilder:default:={"requests":{"cpu":"100m","memory":"128Mi"},"limits":{"cpu":"100m","memory":"128Mi"}}
	Resources corev1.ResourceRequirements `json:"resources,omitempty"`
}

type IngressSpec struct {
	Host string `json:"host"`
}

//+kubebuilder:object:root=true
//+kubebuilder:subresource:status

// VecodexSet is the Schema for the vecodexsets API
type VecodexSet struct {
	metav1.TypeMeta   `json:",inline"`
	metav1.ObjectMeta `json:"metadata,omitempty"`

	Spec VecodexSetSpec `json:"spec,omitempty"`
}

//+kubebuilder:object:root=true

// VecodexSetList contains a list of VecodexSet
type VecodexSetList struct {
	metav1.TypeMeta `json:",inline"`
	metav1.ListMeta `json:"metadata,omitempty"`
	Items           []VecodexSet `json:"items"`
}

func init() {
	SchemeBuilder.Register(&VecodexSet{}, &VecodexSetList{})
}
