export namespace diff {
	
	export class HostDiff {
	    ip: string;
	    hostname: string;
	    status: string;
	    details: string;
	
	    static createFrom(source: any = {}) {
	        return new HostDiff(source);
	    }
	
	    constructor(source: any = {}) {
	        if ('string' === typeof source) source = JSON.parse(source);
	        this.ip = source["ip"];
	        this.hostname = source["hostname"];
	        this.status = source["status"];
	        this.details = source["details"];
	    }
	}

}

export namespace profile {
	
	export class ScanProfile {
	    default_ports: number[];
	    concurrency: number;
	    timeout_ms: number;
	
	    static createFrom(source: any = {}) {
	        return new ScanProfile(source);
	    }
	
	    constructor(source: any = {}) {
	        if ('string' === typeof source) source = JSON.parse(source);
	        this.default_ports = source["default_ports"];
	        this.concurrency = source["concurrency"];
	        this.timeout_ms = source["timeout_ms"];
	    }
	}

}

export namespace results {
	
	export class DeviceInfo {
	    ip: string;
	    isAlive: boolean;
	    hostname: string;
	    mac: string;
	    os?: string;
	    osFamily?: string;
	    deviceType?: string;
	    vendor?: string;
	    openPorts: number[];
	
	    static createFrom(source: any = {}) {
	        return new DeviceInfo(source);
	    }
	
	    constructor(source: any = {}) {
	        if ('string' === typeof source) source = JSON.parse(source);
	        this.ip = source["ip"];
	        this.isAlive = source["isAlive"];
	        this.hostname = source["hostname"];
	        this.mac = source["mac"];
	        this.os = source["os"];
	        this.osFamily = source["osFamily"];
	        this.deviceType = source["deviceType"];
	        this.vendor = source["vendor"];
	        this.openPorts = source["openPorts"];
	    }
	}
	export class HostResult {
	    ip: string;
	    alive: boolean;
	    hostname: string;
	    mac: string;
	    open_ports: number[];
	
	    static createFrom(source: any = {}) {
	        return new HostResult(source);
	    }
	
	    constructor(source: any = {}) {
	        if ('string' === typeof source) source = JSON.parse(source);
	        this.ip = source["ip"];
	        this.alive = source["alive"];
	        this.hostname = source["hostname"];
	        this.mac = source["mac"];
	        this.open_ports = source["open_ports"];
	    }
	}
	export class ScanReport {
	    schemaVersion: string;
	    // Go type: time
	    startTime: any;
	    // Go type: time
	    endTime: any;
	    total: number;
	    alive: number;
	    devices: DeviceInfo[];
	
	    static createFrom(source: any = {}) {
	        return new ScanReport(source);
	    }
	
	    constructor(source: any = {}) {
	        if ('string' === typeof source) source = JSON.parse(source);
	        this.schemaVersion = source["schemaVersion"];
	        this.startTime = this.convertValues(source["startTime"], null);
	        this.endTime = this.convertValues(source["endTime"], null);
	        this.total = source["total"];
	        this.alive = source["alive"];
	        this.devices = this.convertValues(source["devices"], DeviceInfo);
	    }
	
		convertValues(a: any, classs: any, asMap: boolean = false): any {
		    if (!a) {
		        return a;
		    }
		    if (a.slice && a.map) {
		        return (a as any[]).map(elem => this.convertValues(elem, classs));
		    } else if ("object" === typeof a) {
		        if (asMap) {
		            for (const key of Object.keys(a)) {
		                a[key] = new classs(a[key]);
		            }
		            return a;
		        }
		        return new classs(a);
		    }
		    return a;
		}
	}

}

export namespace store {
	
	export class ScanSummary {
	    id: number;
	    start_time: string;
	    end_time: string;
	    target: string;
	    total_hosts: number;
	    alive_hosts: number;
	
	    static createFrom(source: any = {}) {
	        return new ScanSummary(source);
	    }
	
	    constructor(source: any = {}) {
	        if ('string' === typeof source) source = JSON.parse(source);
	        this.id = source["id"];
	        this.start_time = source["start_time"];
	        this.end_time = source["end_time"];
	        this.target = source["target"];
	        this.total_hosts = source["total_hosts"];
	        this.alive_hosts = source["alive_hosts"];
	    }
	}

}

