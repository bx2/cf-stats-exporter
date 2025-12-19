package cfclient

type CFStatusMetric struct {
	Status uint16 `json:"status"`
	Count  uint64 `json:"count"`
}

type CFFetchOptions struct {
	ZoneID    string
	StartDate string
	EndDate   string
	Limit     uint32
}

type cfDimensions struct {
	EdgeResponseStatus uint16 `json:"edgeResponseStatus"`
}

type cfGroup struct {
	Count      uint64       `json:"count"`
	Dimensions cfDimensions `json:"dimensions"`
}

type cfZone struct {
	HTTPRequestsAdaptiveGroups []cfGroup `json:"httpRequestsAdaptiveGroups"`
}

type cfViewer struct {
	Zones []cfZone `json:"zones"`
}

type cfData struct {
	Viewer cfViewer `json:"viewer"`
}

type cfResponse struct {
	Data cfData `json:"data"`
}
